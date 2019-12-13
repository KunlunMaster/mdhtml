//
// Created by  kunlun on 2019/11/26.
//

#include <regex>
#include <array>
#include "Blocker.h"
#include "LineParser.h"
#include <cassert>
#include <iostream>


using namespace std;
using namespace mdht;

Blocker::Blocker(std::string &str, int pos): mtext(move(str)), mpos{pos}{

}
void Blocker::get_result(std::list<std::string> &ret) {
    ret.push_back(mtoken.first);
    ret.push_back(mline->parse_line( mtext));
    ret.push_back(mtoken.second);
}

void Blocker::parse_line(std::list<std::string> *presult) {
    presult->push_back(mline->parse_line(mtext));
    return ;
}

void Blocker::set_anchor_id(const std::string &id_name) {
    mtoken.first += "<a id=\"" + id_name + "\">";
    mtoken.second = "</a>" + mtoken.second;
}

/*
 * 这个函数最关键啊！
 * brother: position 相同。
 * father： position < self_position 
 */
pair<shared_ptr<Blocker>,shared_ptr<Blocker>>
Blocker::find_father_brother(shared_ptr<Blocker> curr, int pos, BlockType type) {
    pair<shared_ptr<Blocker>,shared_ptr<Blocker>> ret{nullptr,nullptr};
    if(curr == nullptr)
        return ret;
    auto f=curr;
    while(f!=nullptr && f->position() > pos){
		 curr->end_block(f->end_token());
         f = f->father();
	}
    if(f==nullptr){
		curr->end_block();
        return ret;
	}
    if(f->position() == pos) {
        ret.second = f;//brother
        ret.first =  f->father(); 
		if(f->type() != type)
			curr->end_block(f->end_token());
    }
    else 
        ret.first = f;
    return ret;
}

shared_ptr<HeaderBlock> HeaderBlock::recognizer(const string &text, shared_ptr<Blocker> curr) {
	if(curr != nullptr && curr->type() == BlockType::quote_block)
			return nullptr;
    regex hder{R"(^\s{0,3}(#{1,6})\s+(.*?)(\s+#*)*\s*$)"};
    smatch m;
    if(!regex_match(text,m, hder))
        return nullptr;
    string t = m[2];
    shared_ptr<HeaderBlock> ret {new HeaderBlock(t,0,m[1].length())};
	assert(m[1].length() > 0 && m[1].length() < 7);
   string pre="<h"+to_string(m[1].length())+">";
    string suf="</h"+to_string(m[1].length())+">";
   if(m[1].length() == 2)
		   suf += "<hr />";
    ret->set_token(make_pair(pre,suf));
    auto fp = find_father_brother(curr, ret->position(), ret->type());
    ret->set_father(fp.first);
    ret->set_brother(fp.second);
    return ret;
}

std::shared_ptr<Blocker> DefaultBlock::recognizer(const std::string &text, std::shared_ptr<Blocker> curr) {
    regex dfreg{R"***((^\s*)(.*))***"};
    smatch m;
    string new_text;
    int pos = 0;
    if(regex_match(text,m, dfreg)){
        new_text = m[2];
        pos = m[1].length();
    }
    else
        new_text = text;
	//code 
	if(curr->text().empty() && pos > 3)
		return CodeBlock::new_code_block(m[1].str(), m[2].str(), curr);
	
    shared_ptr<DefaultBlock> ret {new DefaultBlock(new_text,pos)};
    string pre=m[1].str();
    string suf=m[1].str();
	if(ret->text().empty() && curr->type() == ret->type() && !curr->text().empty()){
			pre += "<p>";
			suf += "</p>";
	}
    ret->set_token(make_pair(pre,suf));
    auto fb = find_father_brother(curr, pos, ret->type());;
    ret->set_father(fb.first);
    ret->set_brother(fb.second);
    return ret;
}

std::shared_ptr<TOCBlock> TOCBlock::recognizer(const std::string &text, std::shared_ptr<Blocker> curr) {
    regex toc_reg{R"(^\s*\[TOC\]\s*$)",regex_constants::icase};
    if(!regex_match(text,toc_reg))
        return nullptr;
    string new_text = "[toc]";
    shared_ptr<TOCBlock> ret {new TOCBlock(new_text,0)};
    string pre = "<div id='toc'>";
    string suf = "</div>";

    ret->set_token(make_pair(pre,suf));
    auto fb = find_father_brother(curr, ret->position(), ret->type());;
    ret->set_father(fb.first);
    ret->set_brother(fb.second);
    return ret;
}
std::string mdht::filt_html_tag(const std::string & text)
{
    regex r{R"((.*?)<(.*?)>(.*?))"};
    string f{R"($1$3)"};
    return regex_replace(text,r,f);
}
void TOCBlock::get_toc_list(const std::vector<std::shared_ptr<Blocker>> &blks) {
    int idx = 0;
    for(auto x : blks) {
        if (x->type() != BlockType::header_block)
            continue;
        auto hd = dynamic_pointer_cast<HeaderBlock>(x);
        string idsrc=filt_html_tag(hd->text());
        string id = "toc_"+to_string(idx++) + idsrc;
        hd->set_anchor_id(id);
        mtoc_list.emplace_back(hd->size(),id, idsrc);
    }
}
array<string, 6> define_head_style(){
    array<string,6> ret;
    for(int i = 0 ; i < 6; ++i){
        string size= to_string((i+1)*20)+"px";
        ret[i] = "style=\"margin-left:" + size + "\"";
    }
    return ret;
}
void TOCBlock::get_result(std::list<std::string> &ret) {
    ret.push_back(mtoken.first);
    array<string, 6> style{define_head_style()};
    for(auto & x: mtoc_list){
        auto size = get<0>(x);
        assert(size > 0 && size < 7);
        string link=" ";
        for(int i = 0; i !=size; ++i) link +="  ";
        link += "<a " + style[size-1] + " href=\"#" + get<1>(x) + "\" >";
        link += mline->parse_line(get<2>(x)) + "</a><br />\n";
        ret.push_back(move(link));
    }
    ret.push_back(mtoken.second);
}

std::shared_ptr<UlistBlock> UlistBlock::recognizer(const std::string &text, std::shared_ptr<Blocker> curr) {
    regex ul_reg{R"(^( *)[-\*\+] (.*?)\s*$)"};
	smatch m;
    if(!regex_match(text,m, ul_reg))
        return nullptr;
	string new_text { m[2].str()};
    shared_ptr<UlistBlock> ret {new UlistBlock(new_text,m[1].length())};
    auto fb = find_father_brother(curr, ret->position(),ret->type()); 
   string pre = m[1].str()+"<li>";
   string suf = "</li>";
   if(fb.first == nullptr)
		fb.first = ret;
   if(fb.second == nullptr || fb.second->type() != ret->type())
		pre = m[1].str()+ "<ul>\n" + pre;

    ret->set_father(fb.first);
    ret->set_brother(fb.second);

   ret->set_token(make_pair(pre,suf));
    return ret;
}

const std::vector<unsigned char> UlistBlock::trigger {'-','+','*'};
const std::vector<unsigned char> OlistBlock::trigger {'1','2','3', '4','5',
'6','7','8','9'};


std::shared_ptr<OlistBlock> OlistBlock::recognizer(const std::string &text, std::shared_ptr<Blocker> curr) {
    regex ul_reg{R"(^( *)[1-9]+\. (.*?)\s*$)"};
	smatch m;
    if(!regex_match(text,m, ul_reg))
        return nullptr;
	string new_text { m[2].str()};
    shared_ptr<OlistBlock> ret {new OlistBlock(new_text,m[1].length())};
    auto fb = find_father_brother(curr, ret->position(), ret->type());
    string pre = m[1].str()+"<li>";
    string suf = "</li>";
	if(fb.first == nullptr)
		fb.first = ret;
	if(fb.second == nullptr || fb.second->type() != ret->type())
		pre = m[1].str()+"<ol>\n " + pre; 

    ret->set_father(fb.first);
    ret->set_brother(fb.second);

   ret->set_token(make_pair(pre,suf));
    return ret;
}
std::shared_ptr<CodeBlock> CodeBlock::new_code_block(const std::string & pretext, const std::string &text, std::shared_ptr<Blocker> curr) {
	string new_text {text};
    shared_ptr<CodeBlock> ret {new CodeBlock(new_text,pretext.size())};
    auto fb = find_father_brother(curr, ret->position(),ret->type());
    string pre = pretext +"<pre><code>\n";
    string suf = pretext +  "</code></pre>";

    ret->set_father(fb.first);
    ret->set_brother(fb.second);

   ret->set_token(make_pair(pre,suf));
    return ret;
}
std::shared_ptr<CodeBlock> CodeBlock::recognizer(const std::string &text, std::shared_ptr<Blocker> curr) {
    regex reg{R"(^( *)`{3}\s*(.*)$)"};
	smatch m;
    if(!regex_match(text,m, reg))
        return nullptr;
	return new_code_block(m[1].str(), m[2].str(), curr);
/*	string new_text {""};
    shared_ptr<CodeBlock> ret {new CodeBlock(new_text,m[1].length())};
    auto fb = find_father_brother(curr, ret->position(),ret->type());
    string pre = m[1].str()+"<pre><code>\n";
    string suf = m[1].str() +  "</code></pre>";

    ret->set_father(fb.first);
    ret->set_brother(fb.second);

   ret->set_token(make_pair(pre,suf));
    return ret;
*/}
void CodeBlock::get_result(std::list<std::string> & ret) 
{    
	ret.push_back(mtoken.first);
    ret.push_back(convert_code(mtext));
    ret.push_back(mtoken.second);
}

bool CodeBlock::add_content(const std::string & text)
{
	if(mis_end) 
		return false;
	regex reg{R"(^( *)`{3}\s*$)"};
    if(regex_match(text,reg)){
		mis_end = true;
        return true;
	}
	regex reg2{R"(^( *)(.*)$)"};
	smatch m;
    if(regex_match(text,m, reg2) && m[1].length() < mpos){
		mis_end = true;
        return false;
	}
	mtext += text.substr(mpos) + "\n";
	return true;
}
/*
   处理 < >  & 的转义 
   < - &lt;
   > - &gt;
   & - &amp;
 */
string CodeBlock::convert_code(const std::string & source)
{

	regex reg{R"([\s\S]*?[<>&]+[\s\S]*)"};
    if(!regex_match(source, reg))
		return source;
	array<string, 256> arr;
	arr['<']="&lt;";
	arr['>']="&gt;";
	arr['&']="&amp;";
	string ret{""};
	for(size_t i = 0 ; i != source.size(); ++i){
		unsigned char idx = (unsigned char)source[i];
		arr[idx].empty()?  (ret += source[i]) : (ret += arr[idx]);
	}
	return ret;
}


std::shared_ptr<TableBlock> TableBlock::recognizer(const std::string & text, std::shared_ptr<Blocker> curr)
{
    regex reg{R"(^( *)(\|.*?)+\|\s*$)"};
	smatch m;
    if(!regex_match(text,m, reg))
        return nullptr;
	string new_text {""};
    shared_ptr<TableBlock> ret {new TableBlock(new_text,m[1].length())};
    auto fb = find_father_brother(curr, ret->position(), ret->type());
    string pre = m[1].str() + "<table>\n";
    string suf = m[1].str() + "</table>";

    ret->set_father(fb.first);
    ret->set_brother(fb.second);

   ret->set_token(make_pair(pre,suf));
   return ret;
}

bool TableBlock::is_table(const string & line1, const string & line2, const string & line3)
{
    regex reg{R"(^ *(\|.*?)+\|\s*$)"};
	regex reg2{R"(^ *(\| *:?-+:? *)+\| *$)"};
//    regex reg2{R"(^ *(|[:]?[-]+)+|\s*$)"};
	if(regex_match(line1, reg) && 
	   regex_match(line2, reg2) && 
	   regex_match(line3, reg)){
		mtable.push_back(line1);
		mtable.push_back(line2);
		mtable.push_back(line3);
		return true;
	}
	return false;
}	

bool TableBlock::add_content(const string & text)
{
	if(mtable.empty() || mis_end)
			return false;
    regex reg{R"(^ *(\|.*?)+\|\s*$)"};
	if(!regex_match(text, reg)){
			mis_end = true;
			return false;
	}
	mtable.push_back(text);
	return true;
}

vector<string> TableBlock::get_theme(const string & line)
{
	vector<string> thvect;
	regex threg{R"(\|(.*?)\|)"};
	smatch m;
	auto b = line.cbegin();
	auto e = line.cend();
	while(regex_search(b,e,m,threg)){
		b = m[0].second -1;
		thvect.push_back(m[1]);
	}
	return thvect;
}

vector<string> TableBlock::get_align()
{
	vector<string> align;
	regex threg{R"(\|(.*?)\|)"};
	smatch m;
	auto b = mtable[1].cbegin();
	auto e = mtable[1].cend();
	while(regex_search(b,e,m,threg)){
		b = m[0].second -1;
		align.push_back(m[1]);
	}

	regex left{R"(^\s*:-+\s*$)"};
	regex right{R"(^\s*-+:\s*$)"};
	regex center{R"(^\s*:-+:\s*$)"};
	for(auto & x : align){
		if(regex_match(x,right))
			x = "right";
		else if(regex_match(x,center))
			x = "center";
		else
			x = "left";
	}
	return align;
}

void TableBlock::get_result(std::list<std::string> & ret) 
{
    ret.push_back(mtoken.first);

	vector<string> theme = get_theme(mtable[0]);
	vector<string> align = get_align();
	size_t n = align.size();
	if(n < theme.size()){
		align.resize(theme.size());
		fill(align.begin()+n, align.end(), "left");
	}
	n = theme.size();
	string preblank = " ";
	for(int i = 0 ; i != mpos;  ++i) preblank += " ";

	//add <th>
	string thstr = preblank + "<thead><tr>\n";
	for(size_t i = 0 ; i != n ; ++i)
		thstr += preblank + " <th align=\"" + align[i] + "\">" + theme[i] + "</th>\n";
	thstr += preblank + "</tr></thead>\n";
	ret.push_back(move(thstr));

	//add <td>
	for(size_t i = 2 ; i != mtable.size(); ++i){
		string tdstr = preblank;
	    if(i%2) tdstr += "<thead>";	
		tdstr += "<tr>\n";
		auto td = get_theme(mtable[i]);
		for(size_t j = 0 ; j != n ; ++j ){
			tdstr += preblank + " <td align=\"" + align[j] + "\">";
			if(j < td.size())
				tdstr += td[j];
			else
				tdstr += " ";
			tdstr += "</td>\n";
		}
		tdstr += preblank ;
		if(i%2) tdstr += "</thead>";
		tdstr += "</tr>\n";
		ret.push_back(move(tdstr));
	}

    ret.push_back(mtoken.second);
}


std::shared_ptr<QuoteBlock> QuoteBlock::recognizer(const std::string & text, std::shared_ptr<Blocker> curr)
{
    regex reg{R"(^( *)(>+)($|( .*$)))"};
	smatch m;
    if(!regex_match(text,m, reg))
        return nullptr;
	string new_text {text};
	int start = m[2].first - text.begin();
	for(int i = start; i != start + m[2].length(); ++i)
		new_text[i]=' ';
	int pos = m[1].length() + m[2].length()-1;
    shared_ptr<QuoteBlock> ret {new QuoteBlock(new_text,pos)};
	ret->set_level(m[2].length());
    auto fb = find_father_brother(curr, ret->position(), ret->type());
	if(fb.first == nullptr)
		fb.first = ret;
	string pre{}, suf{}; 
	if(fb.second == nullptr || (fb.second)->type() != ret->type() ||
	   dynamic_pointer_cast<QuoteBlock>(fb.second)->level() != ret->level())
		pre = m[1].str() + "<blockquote>";
	
    ret->set_token(make_pair(pre,suf));
    ret->set_father(fb.first);
    ret->set_brother(fb.second);
	return ret;
}


void QuoteBlock::get_result(std::list<std::string> & ret) 
{
	if(!mtoken.first.empty())
		ret.push_back(mtoken.first);
//	if(!mtoken.second.empty())
//		ret.push_back(mtoken.second);
}
