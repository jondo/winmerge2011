Some useful scripts:

BuildManualChm.bat
------------------
Batch file for creating HTML help (.chm) manual.


BuildManualWeb.bat
------------------
Batch file for creating manual as separate HTML files.


changelog.py
------------
 Python script creating a changelog from SVN. The script reads a commit log
 from SVN and outputs formatted changelog.

 Usage: python changelog.py [-h] [--help] [-r:n] [--revisions:n]
  Where:
   -h, --help print usage help
   -r:n, --revisions:n output log for last n revisions (default is 100)


CheckMenuResources.vbs
----------------------
Script that checking the menu resources for missing mnemonic chars and description texts.


CheckUnusedResources.vbs
------------------------
Script for creating a list of (hopefully) unused resource IDs.


CheckVCProj.vbs
---------------
Script for checking the file "Merge.vcproj".

CompareProjectFiles.py
----------------------

 This script compares the project files of different versions of VisualStudio
 as well as the list of files on disk and displays the discrepencies.

 Usage: CompareProjectFiles [-h] [-b <basepath>] <from> <to>
  where:
    -h, --help              print this help
    -b, --base-path <ARG>   set the base path to search for files
                            defaults to ../../Src
    <from> and <to>         either of:
                             disk: files on disk
                             2003: VisualStudio 2003 project file
                             2008: VisualStudio 2008 project file

IncludeFileDependencyAnalyzer.py
--------------------------------
 Script for finding all the files included (directly or indirectly) by
 a C/C++ source file.
 
 Usage: IncludeFileDependencyAnalyzer.py <file.i>
  where:
    <file.i>            is the preprocessor output of the compiler for a given C/C++ file.
                        Look at http://c2.com/cgi/wiki?CppDependencyAnalysis to learn how to
                        generate it.

tsvn_patch.py
-------------
 Script for cleaning up TortoiseSVN created patch files to use with GNU patch.

 Usage: python tsvn_patch.py [-b] patchfile
  where:
   -b, --nobak skip creating a backup file of the original patch file


UpdateTranslations.bat
----------------------
Batch file for updating PO template (.pot) file and merging changes to
all PO files.

Remember to call this batch file after every Merge.rc file change!
