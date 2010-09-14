" Vim syntax file
" Language:	OmegaScript
" Maintainer:	Xapian developers <xapian-discuss@lists.xapian.org>
" URL:		http://xapian.org/
" Version:	1.2.3

" To install: place this file in ~/.vim/syntax/omegascript.vim
" and then create or add the following lines, without the "'s commenting
" them out, to ~/.vim/filetype.vim
"if exists("did_load_filetypes")
"  finish
"endif
"augroup filetypedetect
"  au! BufRead,BufNewFile */templates/* setfiletype omegascript
"augroup END

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if !exists("main_syntax")
  if version < 600
    syntax clear
  elseif exists("b:current_syntax")
  finish
endif
  let main_syntax = 'omegascript'
endif

" Include HTML highlighting at the top level
runtime! syntax/html.vim
unlet b:current_syntax

" HTML included in commands.
" Just do very basic highlighting, because it is likely to be only a fragment.
" We can't even sensibly insist on matching angle brackets.
syn match omegaScriptHTML contained +[<>="]+ 

" OmegaScript commands are case sensitive
syn case match

" tag names
syn match omegaScriptError "[${},]"
syn match omegaScriptError2 "[${,]"
syn match omegaScriptLiteral "$[$().]"
syn match omegaScriptArg contained "\$[1-9_]"

syn match omegaScriptCommand "$[a-zA-Z][a-zA-Z]*" contains=omegaScriptCommandKW,omegaScriptCommandKWDeprecated
syn match omegaScriptBegin contained "$[a-zA-Z][a-zA-Z]*{"he=e-1 contains=omegaScriptKW
syn region omegaScriptItem start="$[a-zA-Z][a-zA-Z]*{" end="}" contains=omegaScriptItem,omegaScriptCommand,omegaScriptBegin,omegaScriptSep,omegaScriptLiteral,omegaScriptArg,omegaScriptError2,omegaScriptHtml,omegaScriptComment keepend extend
syn match omegaScriptSep contained ","
syn region omegaScriptCommentBracePair start="{"rs=s+2 end="}" contains=omegaScriptCommentBracePair keepend extend
syn match omegaScriptCommentStart contained "{"
syn region omegaScriptComment matchgroup=omegaScriptCommentStart start="${" end="}" contains=omegaScriptCommentBracePair,omegaScriptComment keepend extend

" All keywords which can take parameters
syn keyword omegaScriptKW contained add addfilter allterms and cgi cgilist date def div env eq field filesize filterterms find freq ge gt highlight hitlist hostname html htmlstrip httpheader if include le length list log lookup lower lt map max min mod mul muldiv ne nice not opt or pack prettyterm range record relevant set setmap setrelevant slice split sub substr topterms transform uniq unpack unstem upper url value

" All keywords which can take no parameters
syn keyword omegaScriptCommandKW contained add allterms collapsed dbname dbsize defaultop error filters fmt hit hitsperpage id last lastpage msize msizeexact now percentage query querydescription queryterms record relevant relevants score setrelevant stoplist suggestion terms thispage time topdoc topterms version weight

" Deprecated keywords
syn keyword omegaScriptCommandKWDeprecated contained freqs

hi link omegaScriptCommand Statement
hi link omegaScriptBegin Statement
hi link omegaScriptItem PreProc
hi link omegaScriptLiteral Special
hi link omegaScriptKW Type
hi link omegaScriptArg Type
hi link omegaScriptCommandKW Type
hi link omegaScriptCommandKWDeprecated Special
hi link omegaScriptSep Normal
hi link omegaScriptError Error
hi link omegaScriptError2 Error
hi link omegaScriptHtml Constant
hi link omegaScriptComment Comment
hi link omegaScriptCommentBracePair Comment
hi link omegaScriptCommentStart Comment

syn sync fromstart

let b:current_syntax = "omegascript"

if main_syntax == 'omegascript'
  unlet main_syntax
endif

" vim: ts=8
