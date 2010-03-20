void screen_counter(char *loc) __attribute__ ((regparm (1)));
void screen_counter(char *loc) {
	int count;
	for (count=-16;count;count+=2)
		loc[count]=' ';
	for (count=1;;count++)
		show_count(count, loc);
}
