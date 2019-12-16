# comment
the utility allows you to comment files <br/>
Usage: comment [flag] FILENAME <br/>
flags: <br/>
-r FILENAME - to read the comment (or without '-r' flag, if comment exists) <br/>
-w FILENAME - to write the comment (if comment already exists, it'll rewrited) <br/>
-a FILENAME - to append to the comment <br />
-d FILENAME - to delete comment associated with file FILENAME <br/>

### how to install 
To compile use `gcc -o comment comment.c`.
Then just copy compiled file to the directory with your commands.
