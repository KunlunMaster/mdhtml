//
// Created by  kunlun on 2019/11/26.
//

#ifndef MDTOHTML_CONFIG_H
#define MDTOHTML_CONFIG_H
#include "LineParser.h"
namespace mdht{
    const std::vector<idx_reg_fmt> regvect{
        std::make_tuple("*A*",R"((^|[^\*])(\*)([^\*]+?)\2($|[^\*]))","$1<em>$3</em>$4"),
        std::make_tuple("**A**",R"((^|[^\*])(\*{2})([^\*]+?)\2($|[^\*]))","$1<strong>$3</strong>$4"),
        std::make_tuple("***A***",R"((^|[^\*])(\*{3})([^\*]+?)\2($|[^\*]))","$1<i><strong>$3</strong></i>$4"),
		std::make_tuple("******", R"(^\*{3,}\s*$)", "<hr />"),
		std::make_tuple("------", R"(^\-{3,}\s*$)", "<hr />"),

		std::make_tuple("# A #", R"(^\s*#\s+(.*?)(\s+#*)*\s*$)", "<h1>$1</h1>"),
		std::make_tuple("## A ##", R"(^\s*#{2}\s+(.*?)(\s+#*)*\s*$)", "<h2>$1</h2><hr />"),
		std::make_tuple("### A ###", R"(^\s*#{3}\s+(.*?)(\s+#*)*\s*$)", "<h3>$1</h3>"),
		std::make_tuple("#### A ###", R"(^\s*#{4}\s+(.*?)(\s+#*)*\s*$)", "<h4>$1</h4>"),
		std::make_tuple("##### A ###", R"(^\s*#{5}\s+(.*?)(\s+#*)*\s*$)", "<h5>$1</h5>"),
		std::make_tuple("###### A ###", R"(^\s*#{6}\s+(.*?)(\s+#*)*\s*$)", "<h6>$1</h6>"),


        std::make_tuple("_A_",R"((^|[^_])(_)([^_<>]+?)\2($|[^_]))","$1<em>$3</em>$4"),
        std::make_tuple("__A__",R"((^|[^_])(_{2})([^_]+?)\2($|[^_]))","$1<strong>$3</strong>$4"),
        std::make_tuple("___A___",R"((^|[^_])(_{3})([^_]+?)\2($|[^_]))","$1<i><strong>$3</strong></i>$4"),

        std::make_tuple("~~A~~",R"((^|[^~])(~{2})([^~]+?)\2($|[^~]))","$1<s>$3</s>$4"),

    };
	const std::string  html_head {"<!doctype html>\n"
					"<html style='font-size:14px !important'>\n"
					"<head>\n"
					"<link rel=\"stylesheet\" type=\"text/css\" href=\"default.css\">\n"
					"<meta charset='UTF-8'><meta name='viewport' content='width=device-width initial-scale=1'>\n"
					"</head>\n<body>\n"
	};
	const std::string  html_end{"</body>\n</html>\n"
	};
	const std::string  html_style {"<style>\n"
				   "blockquote { border-left: 4px solid #dfe2e5; padding: 0 15px; color: #777777; }\n"
				   "blockquote { padding-right: 0; }\n"
				   "thead { background-color: #f8f8f8; }"
				   "table tr th { font-weight: bold; border: 1px solid #dfe2e5; border-bottom: 0; margin: 0; padding: 6px 13px; }\n"
				   "table tr td { border: 1px solid #dfe2e5; margin: 0; padding: 6px 13px; }\n"
				   "code { border: 1px solid #e7eaed; background-color: #f8f8f8; border-radius: 3px; padding: 0; padding: 2px 4px 0px 4px; font-size: 0.9em; }\n"
				  "code { background-color: #f3f4f4; padding: 0 2px 0 2px; }\n"
				   "</style>\n"
	};

}
#endif //MDTOHTML_CONFIG_H
