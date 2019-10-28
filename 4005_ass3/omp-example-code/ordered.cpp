#include <vector>

class FakeFile
{
public:
    int nothing;
    void Compress(){};
};

void Send(FakeFile &f) {}

int main(void)
{
    const unsigned size = 100;
    std::vector<FakeFile> files(size);
#pragma omp for ordered schedule(dynamic)
    for (unsigned n = 0; n < size; ++n)
    {
        files[n].Compress();

#pragma omp ordered
        Send(files[n]);
    }
    return 0;
}
