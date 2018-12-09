# PDFtoText
## Library to transform PDF to plain text files
WORK IN PROGRESS

The state of the repository is not ready for compiling yet. This is rewriting of this [project](https://www.codeproject.com/Articles/7056/Code-to-extract-plain-text-from-a-PDF-file).
It will be a library to include in other project that aims to calculate the average frecuency of each letter in a given text, as a first step
to information theory applications. The goal is, in all cases, learning.

### TODO
- in ProcessOutput(): 
   1. Use iterators for going trhough output string
   2. Change the way text is saved in file using streams instead of putc()
- Clean unused methods and implement the rest
- Include unit testing (include framework and tests)
