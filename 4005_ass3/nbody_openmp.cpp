#include <algorithm>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <fstream>
#include <omp.h>

#define G 10
#define RADIUS 0.25
#define BOUNDARY 2.5
#define WALL 10
#define SIM_DURATION 100
#define TIMEINTERVAL 0.01
#define BODY_MASS 100

#define BH_THETA 0.5

using namespace std;

int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int PLANET_NUM;

const string outputFile = "nbody_openmp.data";

vector<vector<double>> record;

typedef struct Container {
  double x;
  double y;

  Container(double x, double y) : x(x), y(y) {}
  Container() : x(0), y(0) {}

  Container operator+(const Container &b) const {
      return Container(b.x + x, b.y + y);
  }

  Container operator-(const Container &b) const {
      return Container(x - b.x, y - b.y);
  }

  Container operator*(const double &k) const {
      return Container(k * x, k * y);
  }

  double operator*(const Container &b) const {
      return x * b.x + y * b.y;
  }

  Container operator/(const double &k) const {
      return Container(x / k, y / k);
  }

  double distance() const {
      return sqrt(x * x + y * y);
  }

};

typedef struct Planet {
  Container position;
  Container velocity;
  Container acceleration;
  double mass = 0;
};

vector<Planet> nbody;

typedef struct Region {
  Container position;
  double width;
  double height;

  Region(Container position, double w, double h) : position(position), width(w), height(h) {}
  Region() : position(Container()), width(0), height(0) {}
};

struct QTree {
  bool containsPlanet;
  Planet planet;
  Region region;
  QTree *NE;
  QTree *NW;
  QTree *SE;
  QTree *SW;
};

int QTreeChildCount(QTree *t) {
    int count = 0;
#pragma parallel task
    if (t->NE != NULL) {
#pragma omp crical
		count++;
	}
#pragma parallel task
    if (t->NW != NULL) {
#pragma omp crical
		count++;
	}
#pragma parallel task
    if (t->SE != NULL) {
#pragma omp crical
		count++;
	}
#pragma parallel task
    if (t->SW != NULL) {
#pragma omp crical
		count++;
	}
    return count;
}

QTree *QTreeSubdivide(QTree *t, Container position) {
    if (position.x > t->region.position.x + t->region.width / 2) {
        if (position.y > t->region.position.y + t->region.height / 2)
            return t->NE;
        else
            return t->SE;
    } else {
        if (position.y > t->region.position.y + t->region.height / 2)
            return t->NW;
        else
            return t->SW;
    }
}

void QTreeInsert(Planet p, QTree *t) {
    if (t->containsPlanet) {
        if (QTreeChildCount(t) != 0) {
            // sub-quadtree contains four children
            QTree *subdivision = QTreeSubdivide(t, p.position);
            QTreeInsert(p, subdivision);
        } else {
            // sub-quadtree contains one planet without any child
            t->NE = (QTree *) calloc(1, sizeof(QTree));
            t->SE = (QTree *) calloc(1, sizeof(QTree));
            t->NW = (QTree *) calloc(1, sizeof(QTree));
            t->SW = (QTree *) calloc(1, sizeof(QTree));
            (*t->NE).region = Region(
                Container(t->region.position.x + t->region.width / 2, t->region.position.y + t->region.height / 2),
                t->region.width / 2,
                t->region.height / 2
            );
            (*t->SE).region = Region(
                Container(t->region.position.x + t->region.width / 2, t->region.position.y),
                t->region.width / 2,
                t->region.height / 2
            );
            (*t->NW).region = Region(
                Container(t->region.position.x, t->region.position.y + t->region.height / 2),
                t->region.width / 2,
                t->region.height / 2
            );
            (*t->SW).region = Region(
                Container(t->region.position.x, t->region.position.y),
                t->region.width / 2,
                t->region.height / 2
            );
            QTreeInsert(t->planet, QTreeSubdivide(t, t->planet.position));
            QTreeInsert(p, QTreeSubdivide(t, p.position));
        }
    } else {
        // sub-quadtree does not contain any planet
        t->planet = p;
        t->containsPlanet = true;
    }
}

void QTreeBuild(QTree *t) {
    for (int i = 0; i < PLANET_NUM; i++) {
        QTreeInsert(nbody[i], t);
    }
}

void QTreeDelete(QTree *t) {
    if (t != NULL) {
#pragma omp task
        QTreeDelete(t->NE);
#pragma omp task
        QTreeDelete(t->NW);
#pragma omp task
        QTreeDelete(t->SE);
#pragma omp task
        QTreeDelete(t->SW);
        free(t);
    }
}

Planet QTreeCalculateMass(QTree *t) {
    if (t->containsPlanet && (QTreeChildCount(t) == 0))
        return t->planet;
    else if (QTreeChildCount(t) != 0) {
        t->planet.mass = 0;
        t->planet.position = Container(0, 0);
        vector<Planet> cp;
        cp.push_back(QTreeCalculateMass(t->SW));
        cp.push_back(QTreeCalculateMass(t->SE));
        cp.push_back(QTreeCalculateMass(t->NW));
        cp.push_back(QTreeCalculateMass(t->NE));
        for (auto &c: cp) {
            t->planet.mass += c.mass;
            t->planet.position = t->planet.position + (c.position * c.mass);
        }
        t->planet.position = t->planet.position / t->planet.mass;
        return t->planet;
    }
    Planet p;
    p.position = Container(0, 0);
    p.mass = 0;
    return p;
}

Container QTreeForce(Planet current, QTree *t) {
    if (t != NULL && t->containsPlanet) {
        Container d = t->planet.position - current.position;
        double dist = d.distance();
        double a = t->region.width / dist;
        if (!dist)
            return Container();
        else if (dist < RADIUS)
            dist = RADIUS;
        if (!QTreeChildCount(t) || a < BH_THETA)
            return Container((G * d.x) / (pow(dist, 3)), (G * d.y) / (pow(dist, 3))) * t->planet.mass;
        else {
            // short
            Container combination(0, 0);
            vector<Container> cp;
            cp.push_back(QTreeForce(current, t->NE));
            cp.push_back(QTreeForce(current, t->NW));
            cp.push_back(QTreeForce(current, t->SE));
            cp.push_back(QTreeForce(current, t->SW));
            for (auto &c: cp)
                combination = combination + c;
            return combination;
        }
    }
    return Container(0, 0);
}

//no parallel
void recording() {
    vector<double> tmp;
    for (int i = 0; i < PLANET_NUM; i++) {
        tmp.push_back(nbody[i].position.x);
        tmp.push_back(nbody[i].position.y);
    }
    record.push_back(tmp);
}

//no parallel
void randomInitPlanets() {
    vector<Container> candidates;
    for (double i = 0.25 * SCREEN_WIDTH; i < 0.75 * SCREEN_WIDTH; i += RADIUS) {
        for (double j = 0.25 * SCREEN_HEIGHT; j < 0.75 * SCREEN_HEIGHT; j += RADIUS) {
            Container v(i, j);
            candidates.push_back(v);
        }
    }
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(candidates.begin(), candidates.end(), default_random_engine(seed));
    for (int i = 0; i < PLANET_NUM; i++) {
        Planet p;
        p.position = candidates.back();
        p.velocity = Container(0, 0);
        p.acceleration = Container(0, 0);
        p.mass = BODY_MASS;//mass change point
        nbody.push_back(p);
        candidates.pop_back();
    }
}

//no parallel
void collide(Planet &a, Planet &b) {
    Container dab = a.position - b.position;
    Container dba = b.position - a.position;
    Container vab = a.velocity - b.velocity;
    Container vba = b.velocity - a.velocity;
    double distab = dab.distance();
    // momentum stays the same in the question
	// cahnge this id have time (remember to change the mass of the planet)
    // tangent velocity
	Container temp;
	temp = dab * ((vab * dab) / (distab * distab));
    a.velocity = a.velocity - Container(2 * a.mass / (a.mass + b.mass) * temp.x, 2 * a.mass / (a.mass + b.mass) * temp.y);
	temp = dba * ((vba * dba) / (distab * distab));
    b.velocity = b.velocity - Container(2 * b.mass / (a.mass + b.mass) * temp.x, 2 * b.mass / (a.mass + b.mass) * temp.y);
}

//no parallel
void outputToFile() {
    ofstream output(outputFile);
    for (int i = 0; i < SIM_DURATION + 1; i++) {
        for (int j = 0; j < PLANET_NUM * 2; j++)
            output << record[i][j] << " ";
        output << endl;
    }
    output.close();
}

int main(int argc, char **argv) {
	SCREEN_WIDTH = atoi(argv[1]);
	SCREEN_HEIGHT = atoi(argv[2]);
	PLANET_NUM = atoi(argv[3]);
    randomInitPlanets();
//    recording();

    double dtime = omp_get_wtime();

    for (int i = 0; i < SIM_DURATION; i++) {
        QTree *r = (QTree *) calloc(1, sizeof(QTree));
        r->containsPlanet = false;
        r->region = Region(
            Container(),
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
#pragma omp task
{
        QTreeBuild(r);
        QTreeCalculateMass(r);
}
#pragma omp parallel for
		for (int j = 0; j < PLANET_NUM; j++) {
            nbody[j].acceleration = QTreeForce(nbody[j], r) / nbody[j].mass;
            Container v0 = nbody[j].velocity;
            nbody[j].velocity = nbody[j].velocity + nbody[j].acceleration * TIMEINTERVAL;
            nbody[j].position = nbody[j].position + (v0 + nbody[j].velocity) * TIMEINTERVAL / 2;
            for (int k = j + 1; k < PLANET_NUM; k++) {
                Container d = nbody[j].position - nbody[k].position;
                if (d.distance() < BOUNDARY)
                    collide(nbody[j], nbody[k]);
            }
            if (nbody[j].position.x > SCREEN_WIDTH - WALL) {
                nbody[j].position.x = SCREEN_WIDTH - WALL;
                nbody[j].velocity.x = -1 * abs(nbody[j].velocity.x);
            }
            if (nbody[j].position.x < WALL) {
                nbody[j].position.x = WALL;
                nbody[j].velocity.x = abs(nbody[j].velocity.x);
            }
            if (nbody[j].position.y > SCREEN_HEIGHT - WALL) {
                nbody[j].position.y = SCREEN_HEIGHT - WALL;
                nbody[j].velocity.y = -1 * abs(nbody[j].velocity.y);
            }
            if (nbody[j].position.y < WALL) {
                nbody[j].position.y = WALL;
                nbody[j].velocity.y = abs(nbody[j].velocity.y);
            }
        }
		QTreeDelete(r);
//        recording();
    }
    double execution_time = omp_get_wtime() - dtime;
    printf("Execution time: %fs\n", execution_time);
//    outputToFile();
    return 0;
}
