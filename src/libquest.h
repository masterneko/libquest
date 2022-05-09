#pragma once

#include <vector>
#include <string>

namespace libquest
{
    enum question_type
    {
        QUESTION_BASE_CLASS,
        QUESTION_INPUT,
        QUESTION_MULTILINE,
        QUESTION_SELECTION
    };

    class question_t
    {
    protected:
        question_type _type;

    public:
        std::string question_text;

        question_t(std::string question)
            : question_text(question),
              _type(QUESTION_BASE_CLASS)
        {
        }

        virtual std::string run()
        {
            return std::string();
        }

        question_type type()
        {
            return _type;
        }
    };

    class input_t : public question_t
    {
    public:
        std::string default_option;

        input_t(std::string question, std::string default_opt)
            : question_t(question),
              default_option(default_opt)
        {
            _type = QUESTION_INPUT;
        }

        input_t(std::string question)
            : question_t(question)
        {
            _type = QUESTION_INPUT;
        }

        std::string run() override;
    };

    class multiline_t : public question_t
    {
    public:
        std::string default_option;

        multiline_t(std::string question, std::string default_text)
            : question_t(question),
              default_option(default_text)
        {
            _type = QUESTION_MULTILINE;
        }

        multiline_t(std::string question)
            : question_t(question)
        {
            _type = QUESTION_MULTILINE;
        }

        std::string run() override;
    };

    class select_t : public input_t
    {
    public:
        std::vector<std::string> options;

        select_t(std::string question, std::string default_opt, const std::vector<std::string> &opts)
            : input_t(question, default_opt)
        {
            options = opts;
            _type = QUESTION_INPUT;
        }

        select_t(std::string question, const std::vector<std::string> &opts)
            : input_t(question)
        {
            options = opts;
            _type = QUESTION_INPUT;
        }

        std::string run() override;
    };
}
