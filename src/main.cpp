#include "libquest.h"

using namespace libquest;

int main(int argc, char** argv)
{
    std::vector<question_t*> questions =
    {
        new input_t {
            "What is your name?"
        },
        new select_t {
            "What is your favorite color?",
            {
                "red",
                "blue",
                "green"
            }
        },
        new multiline_t {
            "Write some multiline text."
        }
    };

    for(auto& question : questions)
    {
        std::string s = question->run();

        //std::cout << "Answered with " << s << std::endl;
        delete question;
    }
}
