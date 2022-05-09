#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>

#include "libquest.h"

#define STYLE1 "\033[1;32m"
#define STYLE2 "\033[0m\033[1m"
#define STYLE3 "\033[0;36m"
#define STYLE4 "\033[1;36m"
#define STYLE_CLEAR "\033[0m"

static void change_term_style(std::string style)
{
    std::cout << style;
}

static void erase_term_lines(int n)
{
    if (n < 1)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        std::cout << "\x1b[1A"
                  << "\x1b[2K";
    }

    std::cout << "\r";
}

namespace libquest
{
    std::string input_t::run()
    {
        std::string result;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE_CLEAR);
        getline(std::cin, result);

        if (result.empty())
        {
            result = default_option;
        }

        erase_term_lines(1);

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE3);
        std::cout << result << std::endl;
        change_term_style(STYLE_CLEAR);

        return result;
    }

    std::string multiline_t::run()
    {
        std::string result;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text;
        change_term_style(STYLE3);
        std::cout << " [Enter 2 empty lines to finish]\n";
        change_term_style(STYLE_CLEAR);

        int blanks = 0;
        int line_num = 0;

        while (!std::cin.eof())
        {
            std::string line;
            std::getline(std::cin, line);

            if (std::cin.fail())
            {
                break;
            }

            if (line.empty() && line_num == 0)
            {
                result = default_option;

                break;
            }

            if (line.empty())
            {
                blanks++;

                if (blanks == 2)
                {
                    break;
                }
            }
            else
            {
                blanks = 0;
            }

            line_num++;
            result.append(line + "\n");
        }

        if (result.empty())
        {
            result = default_option;
        }

        erase_term_lines(line_num + 2);

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << "\n";
        change_term_style(STYLE3);
        std::cout << result;
        change_term_style(STYLE_CLEAR);

        std::cout.flush();

        return result;
    }

    std::string select_t::run()
    {
        std::string result;
        int selected = 0;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << "\n";
        change_term_style(STYLE_CLEAR);

        auto change_selection = [&](int sel = -1)
        {
            if (sel >= 0)
            {
                erase_term_lines(options.size());
            }

            for (int i = 0; i < options.size(); i++)
            {
                if ((sel == -1 && (options[i] == default_option || (i == 0 && default_option.empty()))) || sel == i)
                {
                    change_term_style(STYLE4);
                    std::cout << "> ";
                }
                else
                {
                    std::cout << "  ";
                }

                std::cout << options[i] << "\n";

                change_term_style(STYLE_CLEAR);
            }
        };

        change_selection();

        static struct termios oldt, newt;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // this is a hack to clear the input buffer
        getchar();

        while (true)
        {
            char a = 0, b = 0, c = 0;

            a = getchar();

            if (a == 27)
            {
                b = getchar();

                if (b != '[')
                {
                    continue;
                }

                c = getchar();
            }

            if (c == 'A')
            {
                if (selected > 0)
                {
                    selected--;
                }
                else
                {
                    selected = options.size() - 1;
                }

                change_selection(selected);
            }
            else if (c == 'B')
            {
                if (selected < options.size() - 1)
                {
                    selected++;
                }
                else
                {
                    selected = 0;
                }

                change_selection(selected);
            }
            else if (a == '\n' && b == 0 && c == 0)
            {
                result = options[selected];

                erase_term_lines(options.size() + 1);

                change_term_style(STYLE1);
                std::cout << "? ";
                change_term_style(STYLE2);
                std::cout << question_text << " ";
                change_term_style(STYLE3);
                std::cout << result << std::endl;
                change_term_style(STYLE_CLEAR);

                break;
            }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return result;
    }
}
