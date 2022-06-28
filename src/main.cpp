#include "libquest.h"
#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <algorithm>

using namespace libquest;

int main(int argc, char** argv)
{
    questionaire_t questions =
    {
        new input_t {
            "What is your name?"
        },
        new select_t {
            "What is your favorite color?",
            {
                "red",
                "green",
                "blue"
            }
        },
        new multiline_t {
            "Write some multiline text."
        },
        new yesno_t {
            "Do you want to enter a new entry?"
        }
    };

    table_t table = 
    {
        {
            column_t {
                "\033[1mNAME\033[0m",
                "\033[1mCOLOUR\033[0m",
                "\033[1mTEXT\033[0m"
            }
        },
        TABLE_BORDER_VERT | TABLE_BORDER_HORIZ | TABLE_HEADER_BORDER,
        rounded_borders
    };


    do
    {
        questions.run();
        table.append_column(column_t { questions.answers[0], questions.answers[1], questions.answers[2] });
    }
    while(questions.answers[3] == "yes");

    table.run();
}
