tldr: run batch file to link and generate binaries, run with ./twoShell

twoShell is a mostly simple Bash-style shell. A 'zsh-lite' of sorts. Almost all commands are outsourced by
executing other programs in new processes.

twoShell supports redirections through >, >>, and <, running programs in the background using &,
and piping between ONLY two programs (that is, a single pipe.) 

twoShell also provides:
Batch Mode:
  Text files can be processed as batch files by running twoShell in the following way -
    ./twoShell filename.txt
  The shell will execute all commands in the given file (seperated by newlines), unless the lines begins with a comment tag,
  indicated by '#'.

history:
  View all previously executed commands
     Users can key UP and DOWN to scroll through the previously executed commands (similar to zsh/Bash).
     Users can press CTRL-C to enter "auto-complete mode." While in autocomplete mode, if the user begins to
     enter a command that is in the history, that command will automatically be supplied to the terminal prompt.
     Users can press CTRL-C again to toggle off the mode.

While in auto-complete mode (or in the midst of UP/DOWN arrowing through history), users can edit their 
commands before executing. Surprisingly, I had to specially implement the ability to delete characters. 

 * Implementation Details:
 * Command history is stored in a homebrew link_list. Implementing "scrolling" with the up/down keys and
 * autocomplete necessitates reading each character as it enters the terminal (before the user presses enter).
 * This was acheived using deep magic from Stack Overflow. This character by character input is managed by 
 * a homebrew String class, found in dstring.h. 
 * Input from stdin is read into a char**, which then necessitates frequent shuffling of char*'s to
 * appropriate variables. Better, I think, would be a linked-list type structure (which I've now made, but
 * switching over is going to have to wait until 3shell). That would allow me to more easily 
 * remove/re-arrange parts of the command as needed.
 * Each shell command is broken up into its base components, confusingly named "command". Here, I use
 * command to mean program name, program flags, and redirect information. For example, if
 * "cat file > out | grep "a" " is read from stdin, we would have two command structs, the first holding
 * "cat file > out", and the second holding "grep "a" ".
 * This design decision stems from a belief that this structure will more easily expanded to allow for
 * any number of programs to be piped together.
 * 
 *  Unexpected Behaviour:
 *  If the user enters auto-complete mode and has a command auto-filled, THEN deletes that entire command,
 *  auto-complete will act as if it is "off" until the user presses enter.
