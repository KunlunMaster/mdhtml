//
// Created by  kunlun on 2019/11/26.
//

#ifndef MDTOHTML_LINEPARSER_H
#define MDTOHTML_LINEPARSER_H
#include <string>
#include <utility>
#include <array>
#include <vector>
#include <variant>
#include <functional>

namespace  mdht {
    using deal_regex_func = std::function<std::string (const std::string &)>;
    using str_func_union = std::variant<std::string,deal_regex_func >;
    using reg_fmt_array=std::array<std::vector<std::pair<std::string, str_func_union >>, 256>;
    using idx_reg_fmt=std::tuple<std::string, std::string, std::string>;

    class LineParser {
    public:
        LineParser(const std::vector<idx_reg_fmt> &replace_list);
        std::string parse_line(const std::string & line)const;
		static std::string link_parser(const std::string & line);

    private:
        reg_fmt_array mreplace;

    };

    class Blocker;
    using recog_block_func = std::function<std::shared_ptr<Blocker> (const std::string &, std::shared_ptr<Blocker>)>;

    class MarkdownParser{
    public:
        MarkdownParser(){ init_parser();}
        bool parse(const std::string & md_file);
        bool output_html(const std::string & html_file);
        void set_css(const std::string & css_file);
    private:
        void init_parser();
    private:
        std::shared_ptr<Blocker> recog_block(const std::string & text, std::shared_ptr<Blocker> curr);
        std::array<std::vector<recog_block_func>, 256> mblk_parsers;
        std::vector<std::shared_ptr<Blocker>>  mblks;
        int mtoc_idx{-1};
    };

}//namespace mdht
#endif //MDTOHTML_LINEPARSER_H
