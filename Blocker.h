//
// Created by  kunlun on 2019/11/26.
//

#ifndef MDTOHTML_BLOCKER_H
#define MDTOHTML_BLOCKER_H
#include <memory>
#include <string>
#include <list>

namespace mdht {
    enum class BlockType{
        header_block, toc_block, code_block, ol_block, ul_block, table_block, quote_block,
        default_block  //当其他模块都不是，则默认是此模块<p></p>
    };
    class LineParser;
    class Blocker{
    public:
        Blocker(std::string & str, int pos);
        virtual void get_result(std::list<std::string> & ret) ;

        int position()const {return mpos;}
        std::string text()const {return mtext;}
        std::shared_ptr<Blocker>  father()const {  return mfather ;}
        std::shared_ptr<Blocker>  brother()const { return mbrother;}

        void set_lineparser(LineParser *pline){mline = pline;}
        void set_father(std::shared_ptr<Blocker> ptr){ mfather=ptr;}
        void set_anchor_id(const std::string & id_name);
        void set_brother(std::shared_ptr<Blocker> ptr){ mbrother=ptr;}
        void set_token(const std::pair<std::string, std::string> & token){mtoken = token;}
		virtual std::string end_token(){return std::string{""};}
        BlockType type()const {return mtype;}
        virtual ~Blocker(){}
		void end_block(const std::string & str=""){ str.empty()? (mtoken.second += end_token()):(mtoken.second += str);}

    protected:
        virtual void parse_line(std::list<std::string> * presult);
        static  std::pair<std::shared_ptr<Blocker>, std::shared_ptr<Blocker>>
        find_father_brother(std::shared_ptr<Blocker> curr, int self_pos, BlockType self_type);
        std::string mtext;
        std::pair<std::string, std::string> mtoken;
        std::shared_ptr<Blocker>  mfather{nullptr};
        std::shared_ptr<Blocker>  mbrother{nullptr};
        int mpos;
        BlockType   mtype;
        LineParser  * mline;
    };

    class HeaderBlock: public Blocker{
    public:
        static const unsigned char trigger='#';
        HeaderBlock(std::string & str, int pos, int size): Blocker(str,pos),msize{size}{mtype = BlockType::header_block;}
        static std::shared_ptr<HeaderBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
        int size(){return msize;}
    private:
        int msize;
    };

	class UlistBlock: public Blocker{
	public:
		static const std::vector<unsigned char> trigger;
		UlistBlock(std::string & str, int pos): Blocker(str,pos){mtype = BlockType::ul_block;}
        static std::shared_ptr<UlistBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
		virtual std::string end_token() override {return "\n</ul>";}
	};

	class OlistBlock: public Blocker{
	public:
		static const std::vector<unsigned char> trigger;
		OlistBlock(std::string & str, int pos): Blocker(str,pos){mtype = BlockType::ol_block;}
        static std::shared_ptr<OlistBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
		virtual std::string end_token() override {return "\n</ol>";}
	};


    std::string filt_html_tag(const std::string & text);
    class TOCBlock: public Blocker{
    public:
        static const unsigned char trigger='[';
        TOCBlock(std::string & str, int pos):Blocker(str,pos){mtype = BlockType::toc_block;}
        static std::shared_ptr<TOCBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
        void get_toc_list(const std::vector<std::shared_ptr<Blocker>> & blks );
        virtual void get_result(std::list<std::string> & ret) ;
    private:
        std::vector<std::tuple<int,std::string, std::string>> mtoc_list;
    };

	class CodeBlock: public Blocker{
	public:
		CodeBlock(std::string & str, int pos):Blocker(str,pos){ mtype = BlockType::code_block;}
        static const unsigned char trigger='`';
        static std::shared_ptr<CodeBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
        virtual void get_result(std::list<std::string> & ret) override ;
		bool add_content(const std::string & input_text);
		bool is_end() { return mis_end; }
		void set_code_name(const std::string & name){ mcode_name = name;}
		static std::string convert_code_line(const std::string & source);
		static std::shared_ptr<CodeBlock> new_code_block(const std::string & pretext, const std::string &text, std::shared_ptr<Blocker> curr);
	private:
		static std::string convert_code(const std::string & source);
		std::string mcode_name;
		bool mis_end{false};
	};

    class DefaultBlock: public Blocker{
    public:
        DefaultBlock(std::string & str, int pos):Blocker(str,pos){mtype = BlockType::default_block;}
        static std::shared_ptr<Blocker> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
    };

	class TableBlock: public Blocker{
	public:
		TableBlock(std::string & str, int pos):Blocker(str,pos){ mtype = BlockType::table_block;}
        static const unsigned char trigger='|';
        static std::shared_ptr<TableBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
        virtual void get_result(std::list<std::string> & ret) override ;
		bool is_table(const std::string & line1, const std::string & line2, const std::string & line3) ;
		bool add_content(const std::string & input_text);
		bool is_end()const  { return mis_end; }
	private:
		std::vector< std::string> get_theme(const std::string & line);
		std::vector< std::string> get_align();
		bool mis_end{false};
		std::vector< std::string > mtable;
	};

	
	class QuoteBlock: public Blocker{
	public:
		QuoteBlock(std::string & str, int pos):Blocker(str,pos){ mtype = BlockType::quote_block;}
        static const unsigned char trigger='>';
        static std::shared_ptr<QuoteBlock> recognizer(const std::string & input_text, std::shared_ptr<Blocker> curr);
        virtual void get_result(std::list<std::string> & ret) override ;

		virtual std::string end_token() override {return "</blockquote>";}
		void set_level(int level) { mlevel = level;}
		int level()const {return mlevel;}
	private:
		int mlevel{0};
	};
}
#endif //MDTOHTML_BLOCKER_H
