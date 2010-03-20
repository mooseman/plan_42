
void show_count(int xcount, char *loc) {
	static int ten = 10;
	int count = xcount;
	int tmp;
	do {
		tmp = count % ten;
		count /= ten;
		tmp += '0';
		loc -= 2;
		*loc = tmp;
		} while (tmp=='0');
}
