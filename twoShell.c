/**
 * twoShell is a mostly simple implementation of a Bash-style shell. That is, very few commands are
 * handled "in house" (in this case, on "cd", "exit", and "history"). All other commands are outsourced by
 * executing other programs in new processes.
 * twoShell supports redirections through >, >>, and <, running programs in the background using &,
 * and piping between ONLY two programs (that is, a single pipe.) 
 * 
 * New Features!
 * Batch Mode: Text files can be processed as batch files by running twoShell in the following way:
 *      ./twoShell filename.txt
 * 
 *      The shell will execute all commands in the given file (seperated by newlines), unless the line
 *      begins with a '#', in which case it will be treated as a comment. If the completion was successful,
 *      twoShell will print to the terminal a list of the executed commands.
 * built in command: history
 *      View all previously executed commands
 * Users can key UP and DOWN to scroll through the previously executed commands (similar to zsh/Bash).
 * Users can press CTRL-C to enter "auto-complete mode." While in autocomplete mode, if the user begins to
 *      enter a command that is in the history, that command will automatically be supplied to the terminal prompt.
 *      Users can press CTRL-C again to toggle off the mode.
 * 
 * Bonus!
 * While in auto-complete mode (or in the midst of UP/DOWN arrowing through history), users can edit their 
 * commands before executing. Surprisingly, I had to specially implement the ability to delete characters.
 *
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
 *
 * Sources:
 * Terminal adjustment functions in helper.h
 * https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
 * -niko
 * Delete the line where the terminal cursor currently is located
 * https://stackoverflow.com/questions/1508490/erase-the-current-printed-console-line
 * - mouviciel
 *
 * @file main.c
 * @author Dakotah Kurtz
 * @version 1
 * @date 2024-04-29
 *
 */

#include <fcntl.h> // file flags
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h>
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // wait
#include <unistd.h>    // fork, execlp
#include <signal.h> // SIGINT

#include "linked_list.h"
#include "dstring.h"
#include "helper.h"


struct command
{
    char **exe;    // command and flags
    char *in;      // input file name
    char *out;     // output file name
    pid_t pid;     // pid of process executing command
    int exe_size;  // 1 + number of flags
    int redir_in;  // flag to indicate <
    int redir_out; // flag to indicate >
    int append;    // flag to indicate >>
};

/*
    Set the standard stream to the given path, with given flags and permissions.
    Returns 1 if redirection was successful, 0 otherwise.
*/
int redir(char *path, int stream, int flags, int permissions);

/*
    Returns a struct "loaded" with the given command, flags, and redirect options. Given one full
    "command". For example, if the user enters "ls -l | grep a", the load function expects a pointer
    to JUST ls -l.
*/
struct command load(char **arr, int size);

void free_command(struct command c);

/*
    Executes a single command, including all flags and redirect options.
*/
void execute(struct command c);

/*
    Sequentially executes the given array of commands, from curr_command to command_count.
*/
void execute_commands(struct command commands[], int curr_command, int command_count);


/*
    Signal handler to turn SIGINT (Ctrl-C) into auto-complete mode toggle
*/
void sig_handler(int);

#define prompt                                 \
    if (!batch_mode)                           \
    {                                          \
        printf("twoShell%s %% ", current_dir); \
    }

static char *fork_err_msg = "forking error";
static char *chdir_err_msg = "chdir error";
static char *pipe_err_msg = "pipe error";
static char *fopen_err_msg = "file open error";

int autcmplt_mode = 0;

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler);

    // ANSI escape characters -- really hope this ports
    const char esc = '\033';
    const char up = 'A';
    const char down = 'B';
    const char delete = 127;

    char *line = NULL;
    char **args = NULL;
    char current_dir[1024];
    dstring *input_string = malloc(sizeof(dstring)); // store new command user is in the process of entering
    input_string->size = 0;

    int batch_mode = 0;
    int args_count = 0;
    int bg = 0;
    int command_count = 0;
    int found = 0;    

    size_t len = 0;
    ssize_t nread;

    getcwd(current_dir, sizeof(current_dir)); // get the current directory for display

    if (argc == 2) // attempt to enter batch mode
    {
        // redirect input from stdin to come from the file
        if (redir(argv[1], 0, O_RDONLY, 0666))
        {
            batch_mode = 1;
        }
        else
        {
            perror("Invalid batch file");
            return -1;
        }
    }
    if (!batch_mode)
    {
        puts(
            " -----------------------------------------------------------------------------------------\n"
            "|                                                                                         |\n"
            "|                                   ********    ***   ***    ********   ***      ***      |\n"
            "|                                  ****         ***   ***    **         ***      ***      |\n"
            "| **********                      ****          ***   ***    **         ***      ***      |\n"
            "|     *      *       *   ****      *****        *********    ******     ***      ***      |\n"
            "|     *      *   *   *  *    *        *****     ***   ***    **         ***      ***      |\n"
            "|     *       * * * *   *    *         ****     ***   ***    **         ***      ***      |\n"
            "|     *        *   *     *****    ********      ***   ***    *******    *******  *******  |\n"
            "|                                                                                         |\n"
            " -----------------------------------------------------------------------------------------\n");

        printf("by Dakotah\n\n");
    }

    // linked_list to store the history
    llist *history_ll = malloc(sizeof(llist));
    history_ll->length = 0;
    history_ll->head = NULL;
    history_ll->tail = NULL;

    while (1)
    {
        fflush(stdout);
        if (batch_mode)
        {
            do
            { // in Batch mode, all input comes directly from the file (stdin)
                if (((nread = getline(&line, &len, stdin)) == EOF))
                {
                    printf("%s batch completed: \n", argv[1]);
                    print(history_ll, '\n');
                    return 0;
                }

            } while (!strcmp(line, "") || (starts_with(line, '#')));
        }
        else
        {
            char c;
            int count = (history_ll->length); // how many commands are in history
            dstring *history_additions[count];
            // allocate dstrings for all history commands to allow users to edit them
            for (int i = 0; i < count; i++)
            {
                dstring *string = malloc(sizeof(dstring));
                history_additions[i] = string;
                history_additions[i]->size = 0;
                char *hist = get(history_ll, i);
                int j = 0;
                // copy the value in history linkedlist to the appropriate history dstring
                while (hist[j] != '\0')
                {
                    add_end(history_additions[i], hist[j]);
                    j++;
                }
            }
            do // actually get the command
            {
                prompt while (1) // until the user enters a command and presses enter
                {
                    c = getch();

                    if (c == delete)
                    {
                        if (count == history_ll->length) // the user is entering a new command
                        {
                            if (input_string->size > 0)
                            {
                                // remove it from the input string, and adjust terminal to show that.
                                remove_dstring_index(input_string, (input_string->size) - 1);
                                putchar(0x8);
                                putc(' ', stdout);
                                putchar(0x8);
                            }
                        }
                        else // the user is deleting something they scrolled to using arrow keys
                        { 
                            if (history_additions[count]->size > 0)
                            {
                                // remove from history dstring, and adjust terminal to show that
                                remove_dstring_index(history_additions[count], (history_additions[count]->size) - 1);
                                putchar(0x8);
                                putc(' ', stdout);
                                putchar(0x8);
                            }
                        }
                    }
                    else if (c == esc) // it's an escape char (arrow keys)
                    {
                        getch(); // skip 'garbage'

                        switch (getch())
                        {
                        case up:
                            if (count != 0) // nothing in history below index 0
                            {
                                count--;
                                /*
                                  - mouviciel
                                  */
                                printf("\33[2K\r"); // Clear entire line, move cursor back to start of line
                                prompt

                                printf("%s", history_additions[count]->arr);
                            }

                            break;

                        case down:

                            printf("\33[2K\r"); // Clear entire line, move cursor back to start of line
                            prompt 
                            if (count < ((history_ll->length))) 
                            // nothing in history beyond the linked_lists length
                            {
                                count++;
                            }

                            if (count < ((history_ll->length)))
                            {

                                printf("%s", history_additions[count]->arr);
                            }
                            else // we've reached the end of the history - display any typing the user has done
                            // on a fresh line
                            {
                                if (input_string->size > 0)
                                {
                                    printf("%s", input_string->arr);
                                }
                            }

                            break;
                        }
                    }
                    else // it's not an escape key, add it to the appropriate dstring
                    {
                        if (count != history_ll->length) // editing history dstring
                        {
                            // only add new line if there is SOMETHING to execute
                            if (!(c == '\n' && history_additions[count]->size == 0))
                            {
                                add_end(history_additions[count], c);
                            }
                        }
                        else if (!(c == '\n' && input_string->size == 0))
                        {
                            add_end(input_string, c);
                        }
                        putc(c, stdout);
                    }
                    // only auto-complete if we're not scrolling history and we haven't already
                    // autofilled.
                    if (autcmplt_mode && input_string->size != 0 && count == history_ll->length && !found)
                    {
                        int loc;
                        if ((loc = contains(history_ll, input_string->arr)) != -1)
                        {
                            found = 1;
                            printf("\33[2K\r"); // Clear entire line, move cursor back to start of line
                            prompt
                            line = get(history_ll, loc); // get matched command from history linkedlist
                            printf("%s", line);
                            int j = 0;
                            // clear the input string and copy the auto-completed command into it
                            if (input_string->size != 0)
                            {
                                clear_string(input_string);
                            }
                            while (1)
                            {
                                if (line[j] == '\0')
                                {
                                    break;
                                }
                                add_end(input_string, line[j]);

                                j++;
                            }
                        }
                    }

                    if (c == '\n')
                    {
                        found = 0; // reset autocomplete
                        if (count != history_ll->length)
                        { // going to run cmd from history, copy history dstring into input_string
                            if (input_string->size > 1)
                            {
                                clear_string(input_string);
                            }
                            for (int i = 0; i < history_additions[count]->size; i++)
                            {
                                add_end(input_string, history_additions[count]->arr[i]);
                            }

                            line = input_string->arr;
                            nread = input_string->size;
                        }
                        else if (input_string->size != 0)
                        {
                            line = input_string->arr;
                            nread = input_string->size;

                        }
                        break;
                    }
                }
                // while the line the user hit enter on contains something other than the prompt
            } while (count == history_ll->length && input_string->size < 1);

            for (int i = 0; i < (history_ll->length) - 1; i++)
            {
                free(history_additions[i]->arr);
            }
        }

        nread--;

        // in some branches, we end up with a new line on the end of the command
        if (line[nread] == '\n')
        {
            line[nread] = '\0';
        }

        // add to history
        add_last(history_ll, clean_string(line, nread));

        args = malloc((nread) * sizeof(char *));
        char *arg = strtok(line, " ");

        while (arg != NULL)
        { // load up args, splitting at spaces
            args[args_count] = malloc(strlen(arg) + 1);
            // track the number of pipes
            if (!strcmp(arg, "|"))
            {
                command_count++;
            }
            strcpy(args[args_count], arg);
            args_count++;
            arg = strtok(NULL, " ");
        }

        command_count++; // command_count = #pipes + 1

        if (!strcmp(args[0], "exit"))
        {
            break;
        }
        else if (!strcmp(args[0], "history"))
        {
            print(history_ll, '\n');
        }
        else if (!strcmp(args[0], "cd"))
        {
            if (0 == chdir(args[1]))
            {
                getcwd(current_dir, sizeof(current_dir));
            }
            else
            {
                perror(chdir_err_msg);
            }
        }
        else
        { // "simple" command ie not built in

            // don't block shell, so set flag and remove ampersand from args
            if (!strcmp(args[args_count - 1], "&"))
            {
                bg = 1;
                args_count--;
                char **temp;
                // like the next three lines...this is messy. I THINK that a linked list would make
                // this as simple as remove(&args, args_count);
                copy_arr(args, &temp, 0, args_count);
                free_arr(&args, args_count);
                copy_arr(temp, &args, 0, args_count);
            }

            // "load" up the command structs, which are the contents of args seperated by pipes.
            // This could be done when parsing stdin, but then I would need to dynamically allocate
            // the struct. Maybe in version 1.1
            struct command commands[command_count];
            int i = 0;
            int j = 0;
            int curr_com_count = 0;
            char **temp;
            for (; i < args_count; i++)
            {
                if (!strcmp(args[i], "|"))
                {
                    copy_arr(args, &temp, j, i);
                    commands[curr_com_count] = load(temp, (i - j));
                    free_arr(&temp, (i - j));
                    curr_com_count++;
                    // kind of like a sliding window, and always skipping the pipe char itself
                    j = i;
                    j++;
                }
            }

            // load the last command (always one more command than pipe)
            copy_arr(args, &temp, j, i);
            commands[curr_com_count] = load(temp, (i - j));
            free_arr(&temp, (i - j));

            pid_t pid = fork();
            if (pid < 0)
            {
                perror(fork_err_msg);
                exit(-1);
            }

            else if (pid == 0)
            {

                execute_commands(commands, 0, command_count);
            }
            else
            {
                if (bg == 0)
                { // no ampersand parsed, shell should block
                    wait(0);
                }
            }

            for (int i = 0; i < command_count; i++)
            {
                free_command(commands[i]);
            }
            curr_com_count = 0;
        }

        args_count = 0;
        nread = 0;
        // len = 0;
        command_count = 0;
        bg = 0;
        clear_string(input_string);

        free_arr(&args, args_count);
    }

    free(line);
    exit(EXIT_SUCCESS);

    return 0;
}

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        // autocmplt = !autocmplt
        autcmplt_mode = autcmplt_mode ? 0 : 1; 
    }
}

void execute_commands(struct command commands[], int curr_command, int command_count)
{

    // only one command left, don't need fork()
    if ((curr_command + 1) == command_count)
    {
        execute(commands[curr_command]);
    }
    else
    {
        if (commands[curr_command].redir_out != 0)
        {
            // will need to run command twice, once for redirect and once for pipe
            pid_t pid = fork();
            if (pid < 0)
            {
                perror(fork_err_msg);
            }
            else if (pid == 0)
            {
                execute(commands[curr_command]);
            }
            // remove redirect out information
            commands[curr_command].redir_out = 0;
            free(commands[curr_command].out);
        } // pipe to next command

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1)
        {
            perror(pipe_err_msg);
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror(fork_err_msg);
        }
        else if (pid == 0)
        {
            close(STDOUT_FILENO); // closing stdout
            dup(pipe_fd[1]);      // replacing stdout with pipe write
            close(pipe_fd[0]);    // closing pipe read
            close(pipe_fd[1]);

            execute(commands[curr_command]);
        }

        close(STDIN_FILENO); // closing stdin
        dup(pipe_fd[0]);     // replacing stdin with pipe read
        close(pipe_fd[1]);   // closing pipe write
        close(pipe_fd[0]);

        execute(commands[++curr_command]);
    }
}

void execute(struct command c)
{
    c.pid = getpid(); // tracking pid for future features

    if (c.redir_in)
    {
        redir(c.in, 0, O_RDONLY, 0666);
    }
    if (c.redir_out)
    {
        if (c.append == 0)
        {
            redir(c.out, 1, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        }
        else
        {
            // don't truncate, append
            redir(c.out, 1, O_CREAT | O_WRONLY | O_APPEND, 0666);
        }
    }

    if ((execvp(c.exe[0], c.exe)) == -1)
    {
        fprintf(stderr, "command %s failed\n", c.exe[0]);
        exit(-1); // important to exit, otherwise child gets lost
    }
}

void free_command(struct command c)
{
    free_arr(&(c.exe), c.exe_size);
    if (c.redir_in != 0)
    {
        free((c.in));
    }
    if (c.redir_out != 0)
    {
        free(c.out);
    }
}
int redir(char *path, int to_close, int flags, int permissions)
{
    int stream = open(path, flags, permissions);
    if (stream == -1)
    {
        perror(fopen_err_msg);
    }
    close(to_close);
    int result = dup(stream);
    if (result == -1)
    {
        perror(fopen_err_msg);
    }
    close(stream);
    if (result == -1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

struct command load(char **arr, int size)
{
    int exel = size; // to track command+flag length
    int inl = 0;     // index of in file
    int outl = 0;    // index of out file

    struct command temp;
    temp.redir_in = 0;
    temp.redir_out = 0;
    temp.append = 0;

    for (int i = 0; i < size; i++)
    {
        if (!strcmp(arr[i], "<"))
        {
            exel = i;
            inl = i + 1;
            temp.redir_in = 1;
        }
        else if (!strcmp(arr[i], ">"))
        {
            outl = i + 1;
            temp.redir_out = i + 1;

            if (exel == size)
            { // if no redirection symbol found yet
                exel = i;
            }
        }
        else if (!strcmp(arr[i], ">>"))
        {
            outl = i + 1;
            temp.redir_out = 1;
            temp.append = 1;
            if (exel == size)
            { // if no redirection symbol found yet
                exel = i;
            }
        }
    }

    temp.exe_size = exel;
    copy_arr(arr, &(temp.exe), 0, exel);
    temp.exe[temp.exe_size] = NULL;

    if (inl != 0)
    {
        temp.in = malloc(sizeof(char *));
        strcpy(temp.in, arr[inl]);
    }
    if (outl != 0)
    {
        temp.out = malloc(sizeof(char *));
        strcpy(temp.out, arr[outl]);
    }

    return temp;
}

