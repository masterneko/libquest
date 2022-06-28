#pragma once

#include <initializer_list>
#include <vector>
#include <string>

namespace libquest
{
    enum question_type
    {
        QUESTION_BASE_CLASS,
        QUESTION_INPUT,
        QUESTION_YESNO,
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

    class questionaire_t
    {
    public:
        std::vector<question_t*> questions;
        std::vector<std::string> answers;

        questionaire_t()
        {
        }

        questionaire_t(std::initializer_list<question_t*> l)
        :
        questions(l)
        {
        }

        questionaire_t(const std::vector<question_t*>& quests)
        :
        questions(quests)
        {
        }

        questionaire_t& operator=(const std::vector<question_t*>& quests)
        {
            questions = quests;

            return *this;
        }

        void run();

        ~questionaire_t()
        {
            for(auto& question : questions)
            {
                delete question;
            }
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

    class yesno_t : public question_t
    {
    public:
        bool default_option;

        yesno_t(std::string question, bool default_opt)
            : question_t(question),
              default_option(default_opt)
        {
            _type = QUESTION_YESNO;
        }

        yesno_t(std::string question)
            : question_t(question),
              default_option(false)
        {
            _type = QUESTION_YESNO;
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
            _type = QUESTION_SELECTION;
        }

        select_t(std::string question, const std::vector<std::string> &opts)
            : input_t(question)
        {
            options = opts;
            _type = QUESTION_SELECTION;
        }

        std::string run() override;
    };

    // Table

    enum
    {
        TABLE_BORDER_HORIZ = 0b1,
        TABLE_BORDER_VERT = 0b10,
        TABLE_HEADER_BORDER = 0b100,
        TABLE_FOOTER_BORDER = 0b1000,
        TABLE_TRAILING_TEXT = 0b10000,
    };

    struct borders_t
    {
        std::wstring top_left;
        std::wstring top_right;
        std::wstring bottom_left;
        std::wstring bottom_right;
        std::wstring horizontal_bar;
        std::wstring vertical_bar;
        std::wstring top_intersection;
        std::wstring bottom_intersection;
        std::wstring left_intersection;
        std::wstring right_intersection;
        std::wstring intersection;

        std::wstring padding_left;
        std::wstring padding_right;
    };

    extern const borders_t modern_borders;
    extern const borders_t rounded_borders;
    extern const borders_t ascii_borders;

    using wcolumn_t = std::vector<std::wstring>;
    using column_t = std::vector<std::string>;
    class table_t
    {
    public:
        std::vector<wcolumn_t> columns;
        int properties;
        const borders_t *borders = &modern_borders;

    private:
        int get_largest_row_len(int row_index) const;
        void set_data(const std::vector<column_t> &data);
        void set_data(const std::vector<wcolumn_t> &data);

    public:
        table_t(int props);
        table_t(int props, const borders_t &b);

        table_t(const std::vector<wcolumn_t> &data, int props);
        table_t(const std::vector<wcolumn_t> &data, int props, const borders_t &b);
        table_t(const std::vector<wcolumn_t> &data);

        table_t(const std::vector<column_t> &data, int props);
        table_t(const std::vector<column_t> &data, int props, const borders_t &b);
        table_t(const std::vector<column_t> &data);

        std::wstring get_at_index(int row_index, int col_index) const;

        void append_column(wcolumn_t col);
        void append_column(column_t col);

        std::wstring to_wstring() const;
        std::string to_string() const;

        void run() const;
    };
}
