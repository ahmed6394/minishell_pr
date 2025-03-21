/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gahmed <gahmed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 12:41:01 by gahmed            #+#    #+#             */
/*   Updated: 2025/03/19 11:57:48 by gahmed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/minishell.h"

char *handle_quotes(const char *input, int *index, char quote_type)
{
    int start = *index + 1;
    int end = start;

    while (input[end] && input[end] != quote_type)
        end++;

    if (input[end] != quote_type)
    {
        fprintf(stderr, "Error: Unclosed quote\n");
        return NULL;
    }
    char *quoted_str = strndup(&input[start], end - start);
    *index = end + 1;
    return quoted_str;
}



char **tokenize_input(char *input)
{
    char **tokens = malloc(sizeof(char *) * MAX_TOKENS);
    if (!tokens) {
        perror("malloc failed for tokens");
        exit(1);
    }
    int i = 0, j = 0;

    while (input[i] && j < MAX_TOKENS - 1)
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;

        if (!input[i]) break;

        if (input[i] == '|')
        {
            tokens[j++] = ft_strdup("|");
            i++;
        }
        else if (input[i] == '"' || input[i] == '\'')
        {
            char *quoted_str = handle_quotes(input, &i, input[i]);
            if (!quoted_str) {
                free(tokens);
                return NULL;
            }
            tokens[j++] = quoted_str;
        }
        else 
        {
            int start = i;
            while (input[i] && input[i] != ' ' && input[i] != '|' && input[i] != '"' && input[i] != '\'')
                i++;
            tokens[j] = strndup(&input[start], i - start);
            j++;
        }
    }
    tokens[j] = NULL;

    // Debug print all tokens
    printf("Tokenized Input:\n");
    for (int k = 0; tokens[k]; k++)
    {
        printf("[%d]: %s\n", k, tokens[k]);
    }

    return tokens;
}


void	ft_free_tab(char **tab)
{
	size_t	i;

	i = 0;
	if (!tab)
		return;
	while (tab[i])
	{
		free(tab[i++]);
	}
	free(tab);
}

char	*get_env(char *key, char **env)
{
	int		i;
	int		j;
	char	*prefix;

	i = 0;
	while (env[i])
	{
		j = 0;
		while (env[i][j] && env[i][j] != '=')
			j++;
		prefix = ft_substr(env[i], 0, j);
		if (ft_strcmp(prefix, key) == 0)
		{
			free(prefix);
			return (env[i] + j + 1);
		}
		free(prefix);
		i++;
	}
	return (NULL);
}

char *get_path(char *cmd, char **env)
{
    int i;
    char *exec;
    char **allpath;
    char *path_part;

    if (access(cmd, F_OK | X_OK) == 0)
        return strdup(cmd);
    char *path_env = get_env("PATH", env);
    if (!path_env)
        return NULL;
    allpath = ft_split(path_env, ':');
    if (!allpath)
        return NULL;
    for (i = 0; allpath[i]; i++)
    {
        path_part = malloc(strlen(allpath[i]) + strlen(cmd) + 2);
        if (!path_part)
            continue;
        sprintf(path_part, "%s/%s", allpath[i], cmd);
        
        if (access(path_part, F_OK | X_OK) == 0)
        {
            free(allpath);
            return path_part;
        }
        free(path_part);
    }
    free(allpath);
    return NULL;
}


// void execute_command(char **tokens, t_shell *shell)
// {
//     pid_t pid;
//     int status;

//     if (!tokens || !tokens[0]) {
//         printf("No command to execute.\n");
//         return;
//     }
//     pid = fork();
//     if (pid == 0)
//     {
//         execute_single_command(tokens, shell);
//         exit(shell->last_exit_status);
//     }
//     else if (pid < 0)
//     {
//         perror("fork failed");
//         shell->last_exit_status = 1;
//         return;
//     }
//     waitpid(pid, &status, 0);
//     if (WIFEXITED(status)) {
//         shell->last_exit_status = WEXITSTATUS(status);
//     }
// }

void execute_command(char **tokens, t_shell *shell)
{
    pid_t pid;
    int status;

    if (!tokens || !tokens[0])
    {
        printf("No command to execute.\n");
        return;
    }
    pid = fork();
    if (pid == 0)
    {
        // execute_single_command(tokens, shell);
		execute_redirection(tokens, shell);
        exit(shell->last_exit_status);
    }
    else if (pid < 0)
    {
        perror("fork failed");
        shell->last_exit_status = 1;
        return;
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
    {
        shell->last_exit_status = WEXITSTATUS(status);
    }
}
