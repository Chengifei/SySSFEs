\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{documentation}[2017/09/17 Documentation for ATOM]

\DeclareOption{draft}{

}
\ProcessOptions

\LoadClass{article}
\RequirePackage[margin=1in]{geometry}
\RequirePackage{fancyhdr}
\pagestyle{fancy}
\lhead{\textcopyright\space Yifei Zheng}
\renewcommand\headrule{}

\newcommand{\NOTE}[1]{[\textit{Note:} #1 —-- end-note]}

\renewcommand\maketitle{
    \begin{titlepage}
    \null\vfil
    \vskip 60pt
    \Huge
    \begin{center}
    \setlength{\baselineskip}{2eM}
    ATOM Documentation \\ and Specifications
    \vskip 1.5eM
    {\large \@date \par}
    \end{center}\par
    \vfil\null
    \end{titlepage}
}


\newcounter{clausecnt}[subsection]
\newcounter{subclausecnt}[clausecnt]

\newlength{\@secindent}
\settowidth{\@secindent}{\normalfont\Large\bfseries\@seccntformat{section}\relax}
\renewcommand\section[2]{
    \@startsection{section}{1}{0pt}{2ex plus 1ex}
    {-1ex plus -0.2ex}{\normalfont\Large\bfseries}{#1}\hfill [#2]
    \@xsect{2ex plus 0.2ex}
    \parindent0pt
    \parskip1ex
}

\renewcommand\subsection[2]{
    \@startsection{subsection}{2}{0pt}{1.5ex plus 1ex minus 0.2ex}
    {-1ex plus -0.2ex}{\normalfont\large\bfseries}{#1}\hfill [#2]
    \@xsect{2ex plus 0.2ex}
    \parindent0pt
    \parskip1ex
}

\newcommand\clause[1]{
    \refstepcounter{clausecnt}
    \def\cnt{\small\theclausecnt\space}
    \settowidth{\@tempdima}{\cnt}
    \hspace{-\@tempdima}{\cnt}#1 \par
}

\newcommand\subclause[1]{
    \refstepcounter{subclausecnt}
    \def\cnt{\theclausecnt.\thesubclausecnt\space}
    \settowidth{\@tempdima}{\cnt}
    \hspace{-\@tempdima}\cnt\hspace{0.5eM} #1 \par
}

\newcommand\term[2]{
    \def\cnt{\textbf{#1}\space}
    \settowidth{\@tempdima}{\cnt}
    \cnt{\hangindent\@tempdima #2 \par}
}

\newcommand\desc[1]{
    [[\textsl{Impl}]] #1 \par
}
