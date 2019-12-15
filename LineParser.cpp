//
// Created by  kunlun on 2019/11/26.
//
#include <regex>
#include <fstream>
#include <iostream>
#include "LineParser.h"
#include "Blocker.h"
#include "config.h"

using namespace std;
using namespace mdht;

mdht::LineParser::LineParser(const std::vector<idx_reg_fmt> &replace_list) {
    for(const auto &x : replace_list){
        assert(!get<0>(x).empty() && get<0>(x)[0] > 0);
        mreplace[get<0>(x)[0]].emplace_back(get<1>(x), get<2>(x));
    }
	//` AAA `
	mreplace['`'].emplace_back("", CodeBlock::convert_code_line);
	mreplace['['].emplace_back("", LineParser::link_parser);
}

string LineParser::link_parser(const string & line)
{
//       std::make_tuple("[A](http://a.b.c)",R"((^|[^\[])\[([^\[]+?)\]\(([^\)]+?)\))","$1<a href=\"$3\">$2</a>"),
//       std::make_tuple("[![A](http://a.b.c \"title\")](http://b.c.d)", R"(\[!\[(.*?)\]\((.+?)(\s+\"(.*?)\")?\s*\)\s*\]\((.+?)\))","<a href=\"$4\"><img src=\"$2\" atl=\"$1\" title='$3'></a>"),
	//1, ![alt](http://abc.com "title")
	regex img_reg{R"(!\[(.*?)\]\((.+?)(\s+\"(.*?)\")?\s*\))"};
	auto s = regex_replace(line, img_reg, "<img src=\"$2\" alt=\"$1\" title='$3' >");

	regex link_reg{R"(\[(.*?)\]\((.*?)\))"};
	return regex_replace(s, link_reg, "<a href='$2' >$1</a>");
}

string mdht::LineParser::parse_line(const string &line) const {
    string newline{line};
    for(size_t i=0; i < newline.size(); ++i){
        unsigned char idx= static_cast<unsigned char>(newline[i]);
        if(mreplace[idx].empty())
            continue;
        string tmpresult=newline;
        for(const auto & x : mreplace[idx]){
            if(auto y=get_if<string>(&x.second))
                tmpresult = regex_replace(tmpresult,regex(x.first),*y);
            else{
                auto f = get<deal_regex_func>(x.second);
                tmpresult = f(tmpresult);
            }
        }
        if(newline == tmpresult)
            continue;
        swap(newline,tmpresult);
        i=0;
    }
    return newline;
}


bool mdht::MarkdownParser::output_html(const std::string &html_file) {
    ofstream html(html_file);
    if(!html){
        cerr<<html_file<<" open failed!\n";
        return false;
    }
    if(mblks.empty()){
        cerr<<"there are not any html now!\n";
        return false;
    }

    LineParser lp{regvect};
    if(mtoc_idx >= 0){
      auto x = dynamic_pointer_cast<TOCBlock>(mblks[mtoc_idx]);
      x->get_toc_list(mblks);
    }
    list<string> result;
    for(auto & x : mblks) {
        x->set_lineparser(&lp);
        x->get_result(result);
        result.push_back("\n");
    }
	html << html_head;
//	html << html_style;
    for(auto x : result)
        html<<x;
	html << html_end;
    html.close();

    return true;
}

bool mdht::MarkdownParser::parse(const std::string &md_file) {
    ifstream in(md_file);
    if(!in){
        cerr<<md_file<<" file open failed!\n";
        return false;
    }
    string line_text;
    shared_ptr<Blocker> curr = nullptr;
	if(!getline(in,line_text))
		return false;

    while(1){
        curr = recog_block(line_text,curr);
        if(curr->type() == BlockType::toc_block)
            mtoc_idx = mblks.size();
		else if( curr->type() == BlockType::code_block){
			shared_ptr<CodeBlock> block = dynamic_pointer_cast<CodeBlock>(curr);
			assert(block != nullptr);
			while(getline(in, line_text) && block->add_content(line_text));
			if(in.good()){
				mblks.push_back(curr);
				continue;
			}
		}
		else if(curr->type() == BlockType::quote_block){
			mblks.push_back(curr);
			line_text = curr->text();
			continue;
		}
		else if( curr->type() == BlockType::table_block){
			shared_ptr<TableBlock> block = dynamic_pointer_cast<TableBlock>(curr);
			string line2, line3;
			auto currg = in.tellg();
			if(getline(in, line2) && getline(in, line3) &&
			   block->is_table(line_text, line2,line3)){
				while(getline(in, line_text) && block->add_content(line_text));
				if(in.good()){
					mblks.push_back(curr);
					continue;
				}
			}
			else {
				curr = DefaultBlock::recognizer(line_text, curr);
				in.clear();
				in.seekg(currg);
			}
		}
        mblks.push_back(curr);
		if(!getline(in,line_text))
			break;
    }
    in.close();
    return true;
}

void mdht::MarkdownParser::init_parser() {
    mblk_parsers[HeaderBlock::trigger].push_back(HeaderBlock::recognizer);
    mblk_parsers[CodeBlock::trigger].push_back(CodeBlock::recognizer);
    mblk_parsers[TOCBlock::trigger].push_back(TOCBlock::recognizer);
    mblk_parsers[TableBlock::trigger].push_back(TableBlock::recognizer);
    mblk_parsers[QuoteBlock::trigger].push_back(QuoteBlock::recognizer);
	for(auto x : UlistBlock::trigger)
    	mblk_parsers[x].push_back(UlistBlock::recognizer);
	for(auto x : OlistBlock::trigger)
    	mblk_parsers[x].push_back(OlistBlock::recognizer);
}

shared_ptr<Blocker> mdht::MarkdownParser::recog_block(const std::string &text, shared_ptr<Blocker> curr) {
    for(size_t i = 0 ; i !=text.size(); ++i) {
        if(text[i] == ' ' || text[i] == '\t')
            continue;
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (mblk_parsers[c].empty())
            break;
        for (const auto &x: mblk_parsers[c]) {
            auto ret = x(text, curr);
            if (ret != nullptr)
                return ret;
            else
                break;
        }
    }

    //不能return nullptr, 默认的模块是other_block
    return DefaultBlock::recognizer(text, curr);
}
