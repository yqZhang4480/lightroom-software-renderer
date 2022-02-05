#include "Samples/input_type.hpp"
#include "Samples/texture.hpp"
#include "Samples/colored_vertex.hpp"
using namespace lightroom;
using namespace std;

int main(int argc, char* argv[])
{
    /// press enter to run next sample
    auto sample1 = sample::InputTypeMain();
    auto sample2 = sample::ColoredVertexMain();
    auto sample3 = sample::TextureMain();
}