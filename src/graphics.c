#include <stddef.h>
#include "graphics.h"
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

		asprintf_checked(&line, "Opzionale: %s", card->opzionale ? "Si" : "No");
		multiline_addline(multiline, line);
		asprintf_checked(&line, "Quando: %s", quandoT_str(card->quando));
		multiline_addline(multiline, line);
		asprintf_checked(&line, "Effetti (%d):", card->n_effetti);
		multiline_addline(multiline, line);

		// add actual effects
		for (int i = 0; i < card->n_effetti; i++) {
			asprintf_checked(&line, "%s -> %s (%s)",
				azioneT_str(card->effetti[i].azione),
				tipo_cartaT_str(card->effetti[i].target_carta),
				target_giocatoriT_str(card->effetti[i].target_giocatori)
			);
			multiline_addline(multiline, line);
		}
	} else {
		for (int i = 1; i < CARD_EFFECTS_HEIGHT; i++)
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

	asprintf_checked(&h_border, ANSI_BLUE "%c%s%c" ANSI_RESET, CARD_CORNER_LEFT, HORIZONTAL_BAR, CARD_CORNER_RIGHT);
	asprintf_checked(&v_border, ANSI_BLUE "%c" ANSI_RESET, CARD_BORDER_VERTICAL);

	len_name = strlen(card->name);
	asprintf_checked(&fmt_name, ANSI_BOLD "%s" ANSI_RESET, card->name);

	len_type = asprintf_checked(&type, "#%s", tipo_cartaT_str(card->tipo));
	asprintf_checked(&fmt_type, ANSI_BG_GREEN "%s" ANSI_RESET, type);

	// compute wrapped description
	init_wrapped(&wrapped_description, card->description, CARD_CONTENT_WIDTH-CARD_PADDING);

	// compute effects
	format_effects(&effects_lines, card);

	// add all lines now
	// apend upper border
	multiline_addline(multiline, h_border);
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

	free_wrap(v_border);
	free_wrap(fmt_name);
	free_wrap(type);
	free_wrap(fmt_type);
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

bool show_cards(cartaT *head) {
	multiline_containerT container;
	init_multiline_container(&container);
	freeable_multiline_textT *cards_info;

	int count = count_cards(head);

	cards_info = (freeable_multiline_textT*)malloc_checked(count*sizeof(freeable_multiline_textT));
	for (int i = 0; i < count; i++)
		init_multiline(&cards_info[i]);

	for (int i = 0; i < count; i++, head = head->next)
		build_card(&cards_info[i], head);

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
	clear_multiline_container(&container);
	return count > 0;
}

int get_max_row_width(cartaT *head) {
	int count = count_cards(head);
	if (!count)
		return CARD_WIDTH;
	int max_row_count = count >= CARDS_PER_ROW ? CARDS_PER_ROW : count % CARDS_PER_ROW;
	return CARDS_HORIZONTAL_SPACING + max_row_count * (CARD_WIDTH + CARDS_HORIZONTAL_SPACING);
}

void show_card_group(cartaT *group, const char *title, const char *title_fmt) {
	char *fmt_title;
	char *l_border = "[", *r_border = "]", *vuoto_msg = "\\\\ vuoto //";
	int borders_width = strlen(l_border)+strlen(r_border);
	int max_group_row_width = get_max_row_width(group);

	asprintf_checked(&fmt_title, title_fmt, title);

	print_centered_lr_boxed_string(fmt_title, strlen(title), l_border, r_border, max_group_row_width-borders_width);
	if (!show_cards(group))
		print_centered_lr_boxed_string(vuoto_msg, strlen(vuoto_msg), "", "\n", max_group_row_width);

	free_wrap(fmt_title);
}
