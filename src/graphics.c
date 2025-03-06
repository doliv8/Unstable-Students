#include <stdio.h>
#include <stddef.h>
#include "graphics.h"
#include "format.h"
#include "constants.h"
#include "structs.h"
#include "enums.h"
#include "utils.h"
#include "string.h"
#include "card.h"

void format_effects(freeable_multiline_textT *multiline, cartaT *card) {
	char *line;
	if (card->n_effetti != 0) {
		// add upper padding
		for (int i = 0; i < MAX_EFFECTS-card->n_effetti; i++)
			multiline_addline(multiline, strdup_checked(""));

		// add effects header
		asprintf_s(&line, "Opzionale: %s", card->opzionale ? "Si" : "No");
		multiline_addline(multiline, line);
		asprintf_s(&line, "Quando: %s", quandoT_str(card->quando));
		multiline_addline(multiline, line);
		asprintf_d(&line, "Effetti (%d):", card->n_effetti);
		multiline_addline(multiline, line);

		// add actual effects
		for (int i = 0; i < card->n_effetti; i++) {
			asprintf_sss(&line, "%s -> %s (%s)",
				azioneT_str(card->effetti[i].azione),
				tipo_cartaT_str(card->effetti[i].target_carta),
				target_giocatoriT_str(card->effetti[i].target_giocatori)
			);
			multiline_addline(multiline, line);
		}
	} else {
		// add padding
		for (int i = 1; i < CARD_EFFECTS_HEIGHT; i++) // skip 1 row reserved to "Nessun effetto!"
			multiline_addline(multiline, strdup_checked(""));
		multiline_addline(multiline, strdup_checked("Nessun effetto!"));
	}
}

void build_card(freeable_multiline_textT *multiline, cartaT *card) {
	char *h_border, *v_border, *fmt_name, *type, *fmt_type;
	int len_name, len_type;
	wrapped_textT wrapped_description;
	freeable_multiline_textT effects_lines;
	init_multiline(&effects_lines);

	h_border = ANSI_BLUE CARD_CORNER_LEFT HORIZONTAL_BAR CARD_CORNER_RIGHT ANSI_RESET;
	v_border = ANSI_BLUE CARD_BORDER_VERTICAL ANSI_RESET;

	len_name = strlen(card->name);
	asprintf_s(&fmt_name, ANSI_BOLD "%s" ANSI_RESET, card->name);

	len_type = asprintf_s(&type, "#%s", tipo_cartaT_str(card->tipo));
	asprintf_s(&fmt_type, ANSI_BG_GREEN "%s" ANSI_RESET, type);

	// compute wrapped description
	wrap_text(&wrapped_description, card->description, CARD_CONTENT_WIDTH-CARD_PADDING);

	// compute effects
	format_effects(&effects_lines, card);

	// add all lines now
	// apend upper border
	multiline_addline(multiline, strdup_checked(h_border));
	// append type
	multiline_addline(multiline, center_boxed_string(fmt_type, len_type, v_border, CARD_CONTENT_WIDTH));
	// append name
	multiline_addline(multiline, center_boxed_string(fmt_name, len_name, v_border, CARD_CONTENT_WIDTH));
	// now append wrapped description
	for (int i = 0; i < wrapped_description.multiline.n_lines; i++) {
		multiline_addline(multiline, center_boxed_string(
			wrapped_description.multiline.lines[i],
			wrapped_description.multiline.lengths[i],
			v_border,
			CARD_CONTENT_WIDTH
		)); // add centered boxed line
	}
	// append spacing for reserved description height
	for (int i = 0; i < CARD_DESCRIPTION_HEIGHT-wrapped_description.multiline.n_lines; i++)
		multiline_addline(multiline, center_boxed_string("", 0, v_border, CARD_CONTENT_WIDTH));
	// now append effects
	for (int i = 0; i < effects_lines.n_lines; i++) {
		multiline_addline(multiline, center_boxed_string(
			effects_lines.lines[i],
			effects_lines.lengths[i],
			v_border,
			CARD_CONTENT_WIDTH
		)); // add centered boxed line
	}
	// append bottom border
	multiline_addline(multiline, strdup_checked(h_border)); // duplicate the existing h_border into the heap

	free_wrap(fmt_type);
	free_wrap(type);
	free_wrap(fmt_name);
	clear_wrapped(&wrapped_description);
	clear_freeable_multiline(&effects_lines);
}

void show_card(cartaT *card) {
	freeable_multiline_textT card_info;
	init_multiline(&card_info);
	build_card(&card_info, card);
	for (int i = 0; i < card_info.n_lines; i++)
		puts(card_info.lines[i]);
	clear_freeable_multiline(&card_info);
}

bool show_cards_restricted(cartaT *head, tipo_cartaT type) {
	freeable_multiline_textT *cards_info;
	int count = count_cards_restricted(head, type);

	cards_info = (freeable_multiline_textT*)malloc_checked(count*sizeof(freeable_multiline_textT));
	for (int i = 0; i < count; i++)
		init_multiline(&cards_info[i]);

	for (int i = 0; i < count; head = head->next) {
		if (match_card_type(head, type))
			build_card(&cards_info[i++], head);
	}

	// actually print the built cards in rows containing CARDS_PER_ROW cards max each
	for (int row = 0; row < (count + CARDS_PER_ROW-1)/CARDS_PER_ROW; row++) {
		for (int y = 0; y < CARD_HEIGHT; y++) {
			for (int i = row*CARDS_PER_ROW; i < MIN((row+1)*CARDS_PER_ROW, count); i++)
				printf("%*s%s", CARDS_HORIZONTAL_SPACING, "", cards_info[i].lines[y]);
			puts(""); // finished printing a line of the current row
		}
		puts(""); // add spacing between each row
	}

	for (int i = 0; i < count; i++)
		clear_freeable_multiline(&cards_info[i]);
	free_wrap(cards_info);
	return count > 0;
}

int get_max_row_width_restricted(cartaT *head, tipo_cartaT type) {
	int count = count_cards_restricted(head, type);
	if (!count)
		return CARD_WIDTH;
	int max_row_count = count >= CARDS_PER_ROW ? CARDS_PER_ROW : count % CARDS_PER_ROW;
	return CARDS_HORIZONTAL_SPACING + max_row_count * (CARD_WIDTH + CARDS_HORIZONTAL_SPACING);
}

void show_card_group_restricted(cartaT *group, const char *title, const char *title_fmt, tipo_cartaT type) {
	char *fmt_title;
	char *l_border = "[", *r_border = "]", *vuoto_msg = "\\\\ vuoto //";
	int borders_width = strlen(l_border)+strlen(r_border);
	int max_group_row_width = get_max_row_width_restricted(group, type);

	asprintf_s(&fmt_title, title_fmt, title);

	puts(""); // spacing
	print_centered_lr_boxed_string(fmt_title, strlen(title), l_border, r_border, max_group_row_width-borders_width);
	if (!show_cards_restricted(group, type))
		print_centered_lr_boxed_string(vuoto_msg, strlen(vuoto_msg), "", "\n", max_group_row_width);

	free_wrap(fmt_title);
}

void show_card_group(cartaT *group, const char *title, const char *title_fmt) {
	show_card_group_restricted(group, title, title_fmt, ALL);
}

/**
 * @brief displays a box showing round info (round number and the name of the player playing this round)
 * 
 * @param game_ctx pointer to the game context
 */
void show_round(game_contextT *game_ctx) {
	char *h_border, *v_border, *round_num_text, *player_turn_text;
	int len_round_num_text, len_player_turn_text;
	freeable_multiline_textT round_banner;
	init_multiline(&round_banner);

	h_border = ANSI_BOLD ANSI_BG_MAGENTA CARD_CORNER_LEFT BANNER_HORIZONTAL_BAR CARD_CORNER_RIGHT ANSI_RESET;
	v_border = ANSI_BOLD ANSI_BG_MAGENTA CARD_BORDER_VERTICAL ANSI_RESET;

	len_round_num_text = snprintf(NULL, 0, "Round numero: %d", game_ctx->round_num);
	asprintf_d(&round_num_text, "Round numero: " ANSI_BOLD "%d" ANSI_RESET, game_ctx->round_num);

	len_player_turn_text = snprintf(NULL, 0, "Turno di: %s!", game_ctx->curr_player->name);
	asprintf_s(&player_turn_text, "Turno di: " PRETTY_USERNAME "!", game_ctx->curr_player->name);

	// build actual banner
	multiline_addline(&round_banner, strdup_checked(h_border));
	multiline_addline(&round_banner, center_boxed_string("", 0, v_border, ROUND_BANNER_CONTENT_WIDTH)); // spacing
	multiline_addline(&round_banner, center_boxed_string(round_num_text, len_round_num_text, v_border, ROUND_BANNER_CONTENT_WIDTH));
	multiline_addline(&round_banner, center_boxed_string(player_turn_text, len_player_turn_text, v_border, ROUND_BANNER_CONTENT_WIDTH));
	multiline_addline(&round_banner, center_boxed_string("", 0, v_border, ROUND_BANNER_CONTENT_WIDTH)); // spacing
	multiline_addline(&round_banner, strdup_checked(h_border));

	puts("");
	for (int i = 0; i < round_banner.n_lines; i++)
		puts(round_banner.lines[i]);
	puts("");

	free_wrap(player_turn_text);
	free_wrap(round_num_text);
	clear_freeable_multiline(&round_banner);
}
