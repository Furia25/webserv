/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ansi_sequence.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/05 22:43:18 by vdurand           #+#    #+#             */
/*   Updated: 2025/10/05 23:26:27 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ANSI_SEQUENCE_H
# define ANSI_SEQUENCE_H

// Text colors
# define ANSI_BLACK	"\033[30m"
# define ANSI_RED	"\033[31m"
# define ANSI_GREEN	"\033[32m"
# define ANSI_YELLOW	"\033[33m"
# define ANSI_BLUE	"\033[34m"
# define ANSI_MAGENTA	"\033[35m"
# define ANSI_CYAN	"\033[36m"
# define ANSI_WHITE	"\033[37m"
# define ANSI_BRIGHT_BLACK	"\033[90m"
# define ANSI_BRIGHT_RED	"\033[91m"
# define ANSI_BRIGHT_GREEN	"\033[92m"
# define ANSI_BRIGHT_YELLOW	"\033[93m"
# define ANSI_BRIGHT_BLUE	"\033[94m"
# define ANSI_BRIGHT_MAGENTA	"\033[95m"
# define ANSI_BRIGHT_CYAN	"\033[96m"
# define ANSI_BRIGHT_WHITE	"\033[97m"

// Background colors
# define ANSI_BG_BLACK	"\033[40m"
# define ANSI_BG_RED	"\033[41m"
# define ANSI_BG_GREEN	"\033[42m"
# define ANSI_BG_YELLOW	"\033[43m"
# define ANSI_BG_BLUE	"\033[44m"
# define ANSI_BG_MAGENTA	"\033[45m"
# define ANSI_BG_CYAN	"\033[46m"
# define ANSI_BG_WHITE	"\033[47m"
# define ANSI_BG_BRIGHT_BLACK	"\033[100m"
# define ANSI_BG_BRIGHT_RED	"\033[101m"
# define ANSI_BG_BRIGHT_GREEN	"\033[102m"
# define ANSI_BG_BRIGHT_YELLOW	"\033[103m"
# define ANSI_BG_BRIGHT_BLUE	"\033[104m"
# define ANSI_BG_BRIGHT_MAGENTA	"\033[105m"
# define ANSI_BG_BRIGHT_CYAN	"\033[106m"
# define ANSI_BG_BRIGHT_WHITE	"\033[107m"

// Text styles
# define ANSI_BOLD	"\033[1m"
# define ANSI_DIM	"\033[2m"
# define ANSI_ITALIC	"\033[3m"
# define ANSI_UNDERLINE	"\033[4m"
# define ANSI_BLINK	"\033[5m"
# define ANSI_REVERSE	"\033[7m"
# define ANSI_HIDDEN	"\033[8m"
# define ANSI_STRIKETHROUGH	"\033[9m"

// Reset
# define ANSI_RESET	"\033[0m"
# define ANSI_RESET_BOLD	"\033[21m"
# define ANSI_RESET_DIM	"\033[22m"
# define ANSI_RESET_ITALIC	"\033[23m"
# define ANSI_RESET_UNDERLINE	"\033[24m"
# define ANSI_RESET_BLINK	"\033[25m"
# define ANSI_RESET_REVERSE	"\033[27m"
# define ANSI_RESET_HIDDEN	"\033[28m"
# define ANSI_RESET_STRIKETHROUGH	"\033[29m"

// Cursor movement
# define ANSI_CURSOR_UP	"\033[%dA"
# define ANSI_CURSOR_DOWN	"\033[%dB"
# define ANSI_CURSOR_FORWARD	"\033[%dC"
# define ANSI_CURSOR_BACK	"\033[%dD"
# define ANSI_CURSOR_NEXT_LINE	"\033[%dE"
# define ANSI_CURSOR_PREV_LINE	"\033[%dF"
# define ANSI_CURSOR_HORIZONTAL_ABS	"\033[%dG"
# define ANSI_CURSOR_POSITION	"\033[%d;%dH"

// Screen & line control
# define ANSI_CLEAR_SCREEN	"\033[2J"
# define ANSI_CLEAR_LINE	"\033[2K"
# define ANSI_CLEAR_LINE_FROM_CURSOR_RIGHT	"\033[0K"
# define ANSI_CLEAR_LINE_FROM_CURSOR_LEFT	"\033[1K"
# define ANSI_CLEAR_SCREEN_FROM_CURSOR_DOWN	"\033[0J"
# define ANSI_CLEAR_SCREEN_FROM_CURSOR_UP	"\033[1J"

// Save & restore cursor
# define ANSI_SAVE_CURSOR	"\0337"
# define ANSI_RESTORE_CURSOR	"\0338"

// Other
# define ANSI_BELL	"\a"
# define ANSI_CARRIAGE	"\r"
# define ANSI_TAB	"\t"

#endif