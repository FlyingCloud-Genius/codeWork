#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <fstream>
#include <omp.h>

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 200
#define G 10
#define PARTICLE_NUM 1000
#define PARTICLE_RADIUS 0.25
#define SAFE_BOUND 2.5
#define SAFE_BOUND_WALL 10
#define SIM_DURATION 100
#define SIM_TIMEINTERVAL 0.01
#define BODY_MASS 100

#define BH_THETA 0.5

using namespace std;

const string outputFile = "nbody2.data";

typedef struct vec {
  double x;
  double y;

  vec(double x, double y) : x(x), y(y) {}
  vec() : x(0), y(0) {}

  vec operator+(const vec &b) const {
      return vec(b.x + x, b.y + y);
  }

  vec operator-(const vec &b) const {
      return vec(x - b.x, y - b.y);
  }

  vec operator*(const double &k) const {
      return vec(k * x, k * y);
  }

  double operator*(const vec &b) const {
      return x * b.x + y * b.y;
  }

  vec operator/(const double &k) const {
      return vec(x / k, y / k);
  }

  double distance() const {
      return sqrt(x * x + y * y);
  }

} vec_t;

typedef struct particle {
  vec_t position;
  vec_t velocity;
  vec_t acceleration;
  double mass = 0;
} particle_t;

vector<particle_t> nbody;

typedef struct region {
  vec_t position;
  double width;
  double height;

  region(vec_t position, double w, double h) : position(position), width(w), height(h) {}
  region() : position(vec_t()), width(0), height(0) {}
} region_t;

struct QTree {
  bool containsBody;
  particle_t Body;
  region_t Region;
  QTree *NE;
  QTree *NW;
  QTree *SE;
  QTree *SW;
};

int QTree_ChildCount(QTree *r) {
    int count = 0;
    if (r->NE != NULL) count++;
    if (r->NW != NULL) count++;
    if (r->SE != NULL) count++;
    if (r->SW != NULL) count++;
    return count;
}

QTree *QTree_Subdivision(QTree *r, vec_t position) {
    if (position.x > r->Region.position.x + r->Region.width / 2) {
        if (position.y > r->Region.position.y + r->Region.height / 2)
            return r->NE;
        else
            return r->SE;
    } else {
        if (position.y > r->Region.position.y + r->Region.height / 2)
            return r->NW;
        else
            return r->SW;
    }
}

void QTree_Insert(particle_t p, QTree *r) {
    if (r->containsBody) {
        if (QTree_ChildCount(r) != 0) {
            // sub-quadtree contains four children
            QTree *subdivision = QTree_Subdivision(r, p.position);
            QTree_Insert(p, subdivision);
        } else {
            // sub-quadtree contains one body without any child
            r->NE = (QTree *) calloc(1, sizeof(QTree));
            r->SE = (QTree *) calloc(1, sizeof(QTree));
            r->NW = (QTree *) calloc(1, sizeof(QTree));
            r->SW = (QTree *) calloc(1, sizeof(QTree));
            (*r->NE).Region = region_t(
                vec_t(r->Region.position.x + r->Region.width / 2, r->Region.position.y + r->Region.height / 2),
                r->Region.width / 2,
                r->Region.height / 2
            );
            (*r->SE).Region = region_t(
                vec_t(r->Region.position.x + r->Region.width / 2, r->Region.position.y),
                r->Region.width / 2,
                r->Region.height / 2
            );
            (*r->NW).Region = region_t(
                vec_t(r->Region.position.x, r->Region.position.y + r->Region.height / 2),
                r->Region.width / 2,
                r->Region.height / 2
            );
            (*r->SW).Region = region_t(
                vec_t(r->Region.position.x, r->Region.position.y),
                r->Region.width / 2,
                r->Region.height / 2
            );
            QTree_Insert(r->Body, QTree_Subdivision(r, r->Body.position));
            QTree_Insert(p, QTree_Subdivision(r, p.position));
        }
    } else {
        // sub-quadtree does not contain any body
        r->Body = p;
        r->containsBody = true;
    }
}

void QTree_Build(QTree *r) {
    for (int i = 0; i < PARTICLE_NUM; i++) {
        QTree_Insert(nbody[i], r);
    }
}

void QTree_Delete(QTree *r) {
    if (r != NULL) {
        QTree_Delete(r->NE);
        QTree_Delete(r->NW);
        QTree_Delete(r->SE);
        QTree_Delete(r->SW);
        free(r);
    }
}

particle_t QTree_CalculateMass(QTree *r) {
    if (r->containsBody && (QTree_ChildCount(r) == 0))
        return r->Body;
    else if (QTree_ChildCount(r) != 0) {
        r->Body.mass = 0;
        r->Body.position = vec_t(0, 0);
        vector<particle_t> cp;
        cp.push_back(QTree_CalculateMass(r->SW));
        cp.push_back(QTree_CalculateMass(r->SE));
        cp.push_back(QTree_CalculateMass(r->NW));
        cp.push_back(QTree_CalculateMass(r->NE));
        for (auto &c: cp) {
            r->Body.mass += c.mass;
            r->Body.position = r->Body.position + (c.position * c.mass);
        }
        r->Body.position = r->Body.position / r->Body.mass;
        return r->Body;
    }
    particle_t p;
    p.position = vec_t(0, 0);
    p.mass = 0;
    return p;
}

vec_t QTree_Force(particle_t probe, QTree *r) {
    if (r != NULL && r->containsBody) {
        vec_t d = r->Body.position - probe.position;
        double dist = d.distance();
        double t = r->Region.width / dist;
        if (!dist)
            return vec_t();
        else if (dist < PARTICLE_RADIUS)
            dist = PARTICLE_RADIUS;
        if (!QTree_ChildCount(r) || t < BH_THETA)
            return vec_t((G * d.x) / (pow(dist, 3)), (G * d.y) / (pow(dist, 3))) * r->Body.mass;
        else {
            // short distance
            vec_t fcombination(0, 0);
            vector<vec_t> cp;
            cp.push_back(QTree_Force(probe, r->NE));
            cp.push_back(QTree_Force(probe, r->NW));
            cp.push_back(QTree_Force(probe, r->SE));
            cp.push_back(QTree_Force(probe, r->SW));
            for (auto &c: cp)
                fcombination = fcombination + c;
            return fcombination;
        }
    }
    return vec_t(0, 0);
}

vector<vector<double>> trace;
void outputToFile() {
    ofstream output(outputFile);
    output << "dim: " << 2 << endl;
    output << "duration: " << SIM_DURATION << endl;
    output << "particle_num: " << PARTICLE_NUM << endl;
    output << "particle_rad: " << PARTICLE_RADIUS << endl;
    for (int i = 0; i < SIM_DURATION + 1; i++) {
        for (int j = 0; j < PARTICLE_NUM * 2; j++)
            output << trace[i][j] << " ";
        output << endl;
    }
    output.close();
}

void recordStep() {
    vector<double> tmp;
    for (int i = 0; i < PARTICLE_NUM; i++) {
        tmp.push_back(nbody[i].position.x);
        tmp.push_back(nbody[i].position.y);
    }
    trace.push_back(tmp);
}

void randomInitBodys() {
    vector<vec_t> candidates;
    for (double i = 0.25 * SCREEN_WIDTH; i < 0.75 * SCREEN_WIDTH; i += PARTICLE_RADIUS) {
        for (double j = 0.25 * SCREEN_HEIGHT; j < 0.75 * SCREEN_HEIGHT; j += PARTICLE_RADIUS) {
            vec_t v(i, j);
            candidates.push_back(v);
        }
    }
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(candidates.begin(), candidates.end(), default_random_engine(seed));
    for (int i = 0; i < PARTICLE_NUM; i++) {
        particle_t p;
        p.position = candidates.back();
        p.velocity = vec_t(0, 0);
        p.acceleration = vec_t(0, 0);
        p.mass = BODY_MASS;
        nbody.push_back(p);
        candidates.pop_back();
    }
}

void collide(particle_t &a, particle_t &b) {
    vec_t dab = a.position - b.position;
    vec_t dba = b.position - a.position;
    vec_t vab = a.position - b.position;
    vec_t vba = b.position - a.position;
    double distab = dab.distance();
    // swap radial velocity of two particles and remain the original
    // tangent velocity
    a.velocity = a.velocity - dab * ((vab * dab) / (distab * distab));
    b.velocity = b.velocity - dba * ((vba * dba) / (distab * distab));
}

int main() {
    randomInitBodys();
//    recordStep();
    double dtime = omp_get_wtime();
    for (int i = 0; i < SIM_DURATION; i++) {
        QTree *r = (QTree *) calloc(1, sizeof(QTree));
        r->containsBody = false;
        r->Region = region_t(
            vec_t(),
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
        QTree_Build(r);
        QTree_CalculateMass(r);
        for (int p = 0; p < PARTICLE_NUM; p++) {
            nbody[p].acceleration = QTree_Force(nbody[p], r) / nbody[p].mass;
            vec_t v0 = nbody[p].velocity;
            nbody[p].velocity = nbody[p].velocity + nbody[p].acceleration * SIM_TIMEINTERVAL;
            nbody[p].position = nbody[p].position + (v0 + nbody[p].velocity) * SIM_TIMEINTERVAL / 2;
            for (int pb = p + 1; pb < PARTICLE_NUM; pb++) {
                vec_t d = nbody[p].position - nbody[pb].position;
                if (d.distance() < SAFE_BOUND)
                    collide(nbody[p], nbody[pb]);
            }
            if (nbody[p].position.x > SCREEN_WIDTH - SAFE_BOUND_WALL) {
                nbody[p].position.x = SCREEN_WIDTH - SAFE_BOUND_WALL;
                nbody[p].velocity.x = -1 * abs(nbody[p].velocity.x);
            }
            if (nbody[p].position.x < SAFE_BOUND_WALL) {
                nbody[p].position.x = SAFE_BOUND_WALL;
                nbody[p].velocity.x = abs(nbody[p].velocity.x);
            }
            if (nbody[p].position.y > SCREEN_HEIGHT - SAFE_BOUND_WALL) {
                nbody[p].position.y = SCREEN_HEIGHT - SAFE_BOUND_WALL;
                nbody[p].velocity.y = -1 * abs(nbody[p].velocity.y);
            }
            if (nbody[p].position.y < SAFE_BOUND_WALL) {
                nbody[p].position.y = SAFE_BOUND_WALL;
                nbody[p].velocity.y = abs(nbody[p].velocity.y);
            }
        }
        QTree_Delete(r);
//        recordStep();
    }
    double execution_time = omp_get_wtime() - dtime;
    printf("Execution time: %fs\n", execution_time);
//    outputToFile();
    return 0;
}