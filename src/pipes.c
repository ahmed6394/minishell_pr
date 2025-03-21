/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gahmed <gahmed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 13:06:41 by gahmed            #+#    #+#             */
/*   Updated: 2025/03/21 11:59:40 by gahmed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/minishell.h"

// void execute_single_command(char **tokens, t_shell *shell)
// {
//     char *cmd_path;

//     if (!tokens || !tokens[0]) {
//         printf("No command to execute.\n");
//         return;
//     }
//     if (ft_strcmp(tokens[0], "env") == 0) {
//         builtin_env(shell->env);
//         shell->last_exit_status = 0;
//         return;
//     }
//     cmd_path = get_path(tokens[0], shell->env);
//     if (!cmd_path) {
//         fprintf(stderr, "minishell: command not found: %s\n", tokens[0]);
//         shell->last_exit_status = 127;
//         return;
//     }
//     if (execve(cmd_path, tokens, shell->env) == -1) {
//         perror("execve failed");
//         shell->last_exit_status = 127;
//         return;
//     }
// }

// void execute_redirection(char **tokens, t_shell *shell)
// {
//     int i = 0;
//     char **cmd = calloc(1024, sizeof(char *));
//     int cmd_index = 0;

//     if (!cmd)
//     {
//         perror("malloc failed");
//         exit(1);
//     }
//     for (int j = 0; tokens[j] != NULL; j++)
//         printf("Token[%d]: %s\n", j, tokens[j]);

//     while (tokens[i] != NULL)
//     {
//         if (tokens[i] && strcmp(tokens[i], "<") == 0)
//         {
//             if (tokens[i + 1] == NULL)
//             {
//                 fprintf(stderr, "minishell: syntax error: missing file for input redirection\n");
//                 free(cmd);
//                 exit(1);
//             }
//             handle_input_redirection(tokens, &i);
//             i++;
//         }
//         else if (tokens[i] && (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0))
//         {
//             if (tokens[i + 1] == NULL)
//             {
//                 fprintf(stderr, "minishell: syntax error: missing file for output redirection\n");
//                 free(cmd);
//                 exit(1);
//             }
//             handle_output_redirection(tokens, &i, strcmp(tokens[i], ">>") == 0);
//             i++;
//         }
//         else if (tokens[i] && strcmp(tokens[i], "<<") == 0)
//         {
//             if (tokens[i + 1] == NULL)
//             {
//                 fprintf(stderr, "minishell: syntax error: missing delimiter for heredoc\n");
//                 free(cmd);
//                 exit(1);
//             }
//             handle_heredoc(tokens[i + 1]);
//             i += 2;
//         }
//         else
//         {
//             cmd[cmd_index++] = strdup(tokens[i]);
//             i++;
//         }
//     }
//     cmd[cmd_index] = NULL;
//     execvp(cmd[0], cmd);
//     perror("execvp failed");
//     for (int j = 0; j < cmd_index; j++)
//         free(cmd[j]);
//     free(cmd);
//     exit(1);
// }

void execute_redirection(char **tokens, t_shell *shell)
{
    int i = 0;
    char **cmd = calloc(1024, sizeof(char *));
    int cmd_index = 0;

    if (!cmd)
    {
        perror("malloc failed");
        exit(1);
    }

    // Detect and handle pipes early
    for (int j = 0; tokens[j] != NULL; j++)
    {
        if (strcmp(tokens[j], "|") == 0)
        {
            // Split into separate commands and execute pipes
            tokens[j] = NULL;
            execute_piped_commands(tokens, shell);
            free(cmd);
            return;
        }
    }

    while (tokens[i] != NULL)
    {
        if (strcmp(tokens[i], "<") == 0)
        {
            if (tokens[i + 1] == NULL)
            {
                fprintf(stderr, "minishell: syntax error: missing file for input redirection\n");
                free(cmd);
                exit(1);
            }
            handle_input_redirection(tokens, &i);
            i++;
        }
        else if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0)
        {
            if (tokens[i + 1] == NULL)
            {
                fprintf(stderr, "minishell: syntax error: missing file for output redirection\n");
                free(cmd);
                exit(1);
            }
            handle_output_redirection(tokens, &i, strcmp(tokens[i], ">>") == 0);
            i++;
        }
        else if (strcmp(tokens[i], "<<") == 0)
        {
            if (tokens[i + 1] == NULL)
            {
                fprintf(stderr, "minishell: syntax error: missing delimiter for heredoc\n");
                free(cmd);
                exit(1);
            }
            handle_heredoc(tokens[i + 1]);
            i += 2;
        }
        else
        {
            cmd[cmd_index++] = strdup(tokens[i]);
            i++;
        }
    }

    cmd[cmd_index] = NULL;
    execvp(cmd[0], cmd);
    perror("execvp failed");
    for (int j = 0; j < cmd_index; j++)
        free(cmd[j]);
    free(cmd);
    exit(1);
}


void execute_piped_commands(char **commands, t_shell *shell)
{
    int i = 0, fd[2], input_fd = 0, j = 0;
    pid_t pid;
	int heredoc_fd = -1;

    while (commands[i])
    {
        char **tokens = tokenize_input(commands[i]);
        if (!tokens || !tokens[0])
        {
            ft_free_tab(tokens);
            i++;
            continue;
        }

		while (tokens[j] != NULL)
        {
            if (strcmp(tokens[j], "<<") == 0 && tokens[j + 1])
            {
                handle_heredoc(tokens[j + 1]); // Write input to a temp file
                heredoc_fd = open("/tmp/minishell_heredoc", O_RDONLY); // Open for reading
                tokens[j] = NULL; // Remove heredoc tokens
                tokens[j + 1] = NULL;
            }
			j++;
        }

        if (commands[i + 1])
        {
            if (pipe(fd) == -1)
            {
                perror("pipe failed");
                ft_free_tab(tokens);
                return;
            }
        }

        pid = fork();
        if (pid == 0)
        {
            if (heredoc_fd != -1)
            {
                dup2(heredoc_fd, STDIN_FILENO);
                close(heredoc_fd);
                heredoc_fd = -1; // Reset heredoc fd
            }
            else if (input_fd != 0) // Normal input redirection
            {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (commands[i + 1]) // If another command follows, redirect stdout to pipe
            {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
            }
            execute_redirection(tokens, shell);
            exit(shell->last_exit_status);
        }
        else if (pid < 0)
        {
            perror("fork failed");
            shell->last_exit_status = 1;
            return;
        }
        if (input_fd != 0) 
            close(input_fd);
        if (commands[i + 1])
        {
            close(fd[1]);
            input_fd = fd[0];
        }
        ft_free_tab(tokens);
        i++;
    }
    while (wait(NULL) > 0);
}

