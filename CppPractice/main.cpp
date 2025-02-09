#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>

#include <conio.h>

using namespace std;
using namespace System;
using namespace System::IO;

// ��������� ����������� ���� ����� ���.
struct order
{
	char address[20];  // ����� ������.
	char kind[20];     // ��� ��������.
	char date[11];     // ���� ����������� ������.
	long weight;       // ��� ������.
	long price;        // ���� ������.
};

// ������������ ������� ������, �������� 
// ������ � ��������� ���� � ���-�� 
// ������� ������������� ����.
struct list
{
	char kind[20];      // ��� ��������
	long count;         // ���������� ������� � ���������.
	long price;         // ��������� ���� �������.
	struct list* next;  // ��������� ���� ������.
	struct list* prev;  // ���������� ���� �������.
};

// ������������ ������ ������ ���� struct order.
struct orders
{
	unsigned int count;    // ���������� ������� ���.
	struct order* values;  // ������ ������� ���.
};

// ��������:
// �������� � ������� (������������ ���� �����)
// ������ ������� orders.
static void print_orders(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::White;
	Console::BackgroundColor = ConsoleColor::Black;
	Console::Clear();
	for (unsigned int i = 0; i < orders.count; i++)
	{
		printf(
			"\n%-20s %-20s %11s %7ld %7ld",
			orders.values[i].address,
			orders.values[i].kind,
			orders.values[i].date,
			orders.values[i].weight,
			orders.values[i].price
		);
	}
	(void)_getch();
}

// ��������:
// �������� � ������� (����� �������)
// ����� � ���������� �����.
static void print_most_expensive_order(struct orders orders)
{
	struct order* best = &orders.values[0];
	for (unsigned int i = 1; i < orders.count; i++)
	{
		if (best->price < orders.values[i].price)
		{
			best = &orders.values[i];
		}
	}
	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::BackgroundColor = ConsoleColor::Black;

	Console::CursorLeft = 10;
	printf("����� ������� �����: %ld ���.\n", best->price);
	Console::CursorLeft = 10;
	printf("��� �����: %s\n", best->address);
	(void)_getch();
}

// ��������:
// ��������� ���� �� ������� '����_��_��' (from)
// � ������ '�� ����� ����' (to).
// 
// ���� ������� ������ to � from ������������, 
// �� ��������� ������� ������������.
// 
// ���������:
// to   -> ��������� �� ������ (24+ ����.), 
//         � ������� ����� ��������� ���� � 
//         �������� ������� ��� 
//         ����-��������������� ������.
// from -> ��������� �� ����-��������������� 
//         ������ (11 ����. ������� ����),
//         � ������� ���������� ���� � 
//         ����������� �������.
static void to_pretty_date(char* to, const char* from)
{
	char tmp[3];
	const char* months[] = {
		"������","�������","�����","������","���","����",
		"����","�������","��������","�������","������","�������"
	};
	strcpy(to, from + 8);
	strcat(to, " ");
	strncpy(tmp, from + 5, 2);
	tmp[2] = '\0';
	strcat(to, months[atoi(tmp) - 1]);
	strcat(to, " ");
	strncat(to, from, 4);
}

// ��������:
// �������� � ������� (����� �������)
// ����� � ����� ������� �����.
//
// ���� ������� � ����� ����� ���������, 
// �� �������� ����� �� ���.
static void last_order(struct orders orders)
{
	struct order* best = &orders.values[0];
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (strcmp(orders.values[i].date, best->date) < 0)
		{
			best = &orders.values[i];
		}
	}

	char pretty_date[17];
	to_pretty_date(pretty_date, best->date);
	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::BackgroundColor = ConsoleColor::Black;
	Console::CursorLeft = 10;
	printf("��������� �����:\n");
	Console::CursorLeft = 12;
	printf("������: %s\n", pretty_date);
	Console::CursorLeft = 12;
	printf("�����: % s\n", best->address);
	Console::CursorLeft = 12;
	printf("���: %s\n", best->kind);
	Console::CursorLeft = 12;
	printf("�����: %ld\n", best->price);
	(void)_getch();
}

// ��������:
// �������� � ������� (����� �������)
// ���������� ������� �����.
static void print_pizza_order_count(struct orders orders)
{
	unsigned int total = 0;
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (strcmp(orders.values[i].kind, "�����") == 0) total++;
	}

	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::BackgroundColor = ConsoleColor::Black;

	Console::CursorLeft = 10;
	printf("\n���������� ������� �����: %u", total);
	(void)_getch();
}

// ��������:
// �������� ������ � ���������� ����� ���� ������,
// � ������� ����������� �� ��������� � ����������
// ����.
//
// ���������:
// order -> ������������ ����� ������� �����
//          ����������� �����.
// 
// ����������:
// ����� ���� ������. ����� ���������� ������
// ����� ������������ free().
static struct list* create_list_node(struct order order)
{
	struct list* node = (struct list*)malloc(sizeof(struct list));
	if (!node) abort();
	strcpy(node->kind, order.kind);
	node->count = 1;
	node->price = order.price;
	node->next = NULL;
	node->prev = NULL;
	return node;
}

// ��������:
// ��������� ����� � ������ sp �������� ��� �� ���� 
// ������, ��� ���������� ������� ������.
//
// ������� �����������, ��� sp ������ ����� 
// ��������� �� ������ ������.
//
// ���� ������� ���������� ������ sp ��� �������,
// �� ��������� ������� ������������.
//
// ���������:
// order -> �����, ������� ����� �������� � ������ sp.
static void insert_order(struct list** sp, struct order order)
{
	if (!*sp)
	{
		*sp = create_list_node(order);
		return;
	}

	struct list* node = *sp;
	while (true)
	{
		int cmp = strcmp(order.kind, node->kind);
		if (cmp == 0)
		{
			node->count++;
			node->price += order.price;
			return;
		}
		else if (cmp > 0)
		{
			if (!node->next)
			{
				node->next = create_list_node(order);
				node->next->prev = node;
				return;
			}
			node = node->next;
		}
		else
		{
			struct list* tmp = node->prev;
			node->prev = create_list_node(order);
			node->prev->next = node;
			node->prev->prev = tmp;
			if (tmp) tmp->next = node->prev;
			if ((*sp)->prev) *sp = (*sp)->prev;
			return;
		}
	}
}

// ��������:
// �������� � ������� (������������ ���� �����)
// ������ ������ sp � ���������� ������� 
// (���������� ���� �� ���� ������).
static void list_alpha(struct list* sp)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("���������� ������ �� ���� ������.");
	printf("\n\r----------------------------------------------------------------");
	for (struct list* nt = sp; nt != NULL; nt = nt->next)
	{
		printf(
			"\n\r%-20s %7ld ��. %7ld �.",
			nt->kind,
			nt->count,
			nt->price
		);
	}
	(void)_getch();
}

// ��������:
// �������� � ������� (������������ ���� �����)
// ������ ������ sp � �������� ���������� �������
// (���������� ���� �� ���� ������).
static void list_alpha_reverse(struct list* sp)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("�������� ���������� ������ �� ���� ������.");
	printf("\n\r----------------------------------------------------------------------------");
	struct list* tail = sp;
	while (tail->next != NULL) tail = tail->next;
	for (struct list* nt = tail; nt != NULL; nt = nt->prev)
	{
		printf(
			"\n\r%-20s %7ld ��. %7ld �.",
			nt->kind,
			nt->count,
			nt->price
		);
	}
	(void)_getch();
}

// ��������:
// �������� � ������� (������������ ���� �����)
// ������ ������� �� ������� orders, ���� 
// ������� ��������� 1000.
static void print_expensive_list(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("������ ������� �� ����� ����� 1.000 ���");
	printf("\n\r----------------------------------------------------------------");
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (orders.values[i].price > 1000)
		{
			printf(
				"\n\r%-20s %-20s %7ld � %7ld �.",
				orders.values[i].address,
				orders.values[i].kind,
				orders.values[i].weight,
				orders.values[i].price
			);
		}
	}
	(void)_getch();
}

// ��������:
// �������� � ������� (������������ ���� �����)
// ��������� �� �������� ������� �������,
// ���������� �� ������ ������ sp.
static void diagram(struct orders orders, struct list* sp)
{
	Console::BackgroundColor = ConsoleColor::Yellow;
	Console::Clear();
	long total = 0;
	for (unsigned int i = 0; i < orders.count; i++)
	{
		total += orders.values[i].price;
	}

	ConsoleColor color = ConsoleColor::Black;
	int ncolor = 0;
	for (struct list* nt = sp; nt != NULL; nt = nt->next, ncolor++, color++)
	{
		char str[20];
		sprintf(str, "%3.1f%%", (nt->price * 100.0 / total));
		Console::ForegroundColor = ConsoleColor::Black;
		Console::BackgroundColor = ConsoleColor::Yellow;
		Console::CursorLeft = 5;
		printf("%20s %7ld ��. %20s", nt->kind, nt->count, str);
		Console::BackgroundColor = color;
		Console::CursorLeft = 60;
		printf("%*.s\n", nt->price * 30 / total, "");
		if (ncolor == 13)
		{
			color = ConsoleColor::Black;
			ncolor = 0;
		}
	}
	(void)_getch();
	return;
}

// ��������:
// �������� � ������� (����� �������)
// ������ ���������� ��� ������ ���
// ���� ������� �� ������� orders � 
// ������� ������.
static void match_one(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::BackgroundColor = ConsoleColor::Black;

	struct order a, b;
	bool found_match = false;
	for (unsigned int i = 0; i < orders.count; i++)
	{
		for (unsigned int j = i + 1; j < orders.count; j++)
		{
			a = orders.values[i];
			b = orders.values[j];
			if (strcmp(a.kind, b.kind) == 0) continue;
			if (a.price != b.price) continue;
			found_match = true;
			goto finish;
		}
	}
finish:
	Console::CursorLeft = 10;
	printf("������ ������� � ����������� �����:\n");
	Console::CursorLeft = 10;
	if (found_match)
	{
		printf("����: %7ld\n", a.price);
		Console::CursorLeft = 10;
		printf("��� 1: %-20s\n", a.kind);
		Console::CursorLeft = 10;
		printf("��� 2: %-20s\n", b.kind);
	}
	else printf("���������� �� �������.\n");
	(void)_getch();
}

// ��������:
// �������� � ������� (������������ ���� �����)
// ��� ���������� ��� ������ ���
// ���� ������� �� ������� orders � 
// ������� ������.
static void match(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("������ ��������� � ����������� ����� � ������ �����:");
	for (unsigned int i = 0; i < orders.count; i++)
	{
		for (unsigned int j = i + 1; j < orders.count; j++)
		{
			struct order a = orders.values[i];
			struct order b = orders.values[j];
			if (strcmp(a.kind, b.kind) == 0) continue;
			if (a.price != b.price) continue;
			printf("\r\n%-20s %-20s %7ld", a.kind, b.kind, a.price);
		}
	}
	(void)_getch();
}

// ��������:
// ������������ ���� � ��������� ������� 
// ���� �� ������� ���������� ��� ������
// ����������.
static void menu(struct orders orders, struct list* sp)
{
	const char* options[] = {
		"                                      ",
		"����� ������ �������� ������          ",
		"��������� �����                       ",
		"���������� ������ �� ����             ",
		"���������� ������ �� ���� (��������)  ",
		"������ �� ����� ����� 1.000 �.        ",
		"���������� ������� �����              ",
		"���������                             ",
		"�������� � ����������� �����          ",
		"�������� � ����������� ����� (������) ",
		"�������� ������                       ",
		"�����                                 ",
		"                                      "
	};
	int options_count = sizeof(options) / sizeof(options[0]);
	int first_option = 1;
	int last_option = options_count - 2;

	Console::BackgroundColor = ConsoleColor::Yellow;
	Console::Clear();

	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	for (int i = 0; i < options_count; i++)
	{
		Console::CursorLeft = 10;
		Console::CursorTop = 4 + i;
		printf(" %s ", options[i]);
	}

	int y = 1;
	int last_y = y;


	const int enter = 13;
	const int esc   = 27;
	const int up    = 72;
	const int down  = 80;
	const int home  = 71;
	const int end   = 79;

	while (1)
	{
		Console::ForegroundColor = ConsoleColor::Black;
		Console::BackgroundColor = ConsoleColor::Gray;
		Console::CursorLeft = 10 + 1;
		Console::CursorTop = 4 + last_y;
		printf("%s", options[last_y]);
		Console::ForegroundColor = ConsoleColor::Black;
		Console::BackgroundColor = ConsoleColor::Yellow;
		Console::CursorLeft = 10 + 1;
		Console::CursorTop = 4 + y;
		printf("%s", options[y]);

		Console::CursorTop = 4 + options_count;
		last_y = y;
		switch (_getch())
		{
		case down:  y++; break;
		case up:    y--; break;
		case home:  y = first_option; break;
		case end:   y = last_option; break;
		case enter:
			switch (y)
			{
			case 1: print_most_expensive_order(orders); return;
			case 2: last_order(orders);                 return;
			case 3: list_alpha(sp);                     return;
			case 4: list_alpha_reverse(sp);             return;
			case 5: print_expensive_list(orders);       return;
			case 6: print_pizza_order_count(orders);    return;
			case 7: diagram(orders, sp);                return;
			case 8: match(orders);                      return;
			case 9: match_one(orders);                  return;
			case 10: print_orders(orders);              return;
			case 11: exit(EXIT_SUCCESS);
			}
		case esc:    exit(EXIT_SUCCESS);
		}

		if (y > last_option) y = first_option;
		if (y < first_option) y = last_option;
	}
}

// �������-��������, ����� ������ ��������� ���� � �������.
static void make_file(const char* path)
{
	FILE* file = fopen(path, "w");
	if (fprintf(
		file,
		"15\n"
		"��_������_�10 ������        2024-11-12 5000 2000\n"
		"��_�������_�3 �����         2024-11-11 500  2500\n"
		"��_�������_�3 �������_����� 2024-11-12 250  500\n"
		"��_�������_�2 ����������    2024-11-12 100  400\n"
		"�_����������� �����         2024-11-11 950  6500\n"
		"��_����_�6    ����          2024-11-10 500  2500\n"
		"��_������_�3  �������       2024-11-11 350  2000\n"

		"������        �������       2024-12-11 1000 10000\n"

		"��_������_�12 �������       2024-11-12 2250 250\n"
		"��_�������_�7 �����         2024-11-11 500  2500\n"
		"��_�������_�4 ���_�������   2024-11-12 250  500\n"
		"��_�������_�7 ����������    2024-11-12 100  400\n"
		"��_�������_�9 �������_����� 2024-11-11 100  400\n"
		"��_����_�8    �������       2024-11-13 250  1250\n"
		"��_������_�5  ����          2024-11-11 700  5000\n"
	) < 0) exit(EXIT_FAILURE);
	fclose(file);
}

int main(array<System::String^>^ args)
{
	setlocale(LC_CTYPE, "Russian");
	Console::CursorVisible::set(false);
	Console::BufferHeight = Console::WindowHeight;
	Console::BufferWidth = Console::WindowWidth;

	const char* path = "orders.dat";
	make_file(path);
	FILE* file = fopen(path, "r");
	if (file == NULL)
	{
		printf(
			"\n������: �� ������� ������� ���� %s "
			"��� ������: %s", path, strerror(errno)
		);
		(void)_getch();
		exit(EXIT_FAILURE);
	}

	struct orders orders = {0};

	if (fscanf(file, "%u", &orders.count) != 1)
	{
		printf(
			"\n������: ���� ������ ���������� "
			"����������� ������� (������������� �����)"
		);
		(void)_getch();
		exit(EXIT_FAILURE);
	}

	if (orders.count == 0)
	{
		printf("\n������: ���������� ������� �� ����� ���� �������");
		(void)_getch();
		exit(EXIT_FAILURE);
	}

	orders.values = (struct order*)malloc(sizeof(struct order) * orders.count);
	if (orders.values == NULL) abort();
	FILE* output = fopen("output.dat", "w");
	struct list* sp = NULL;  // ������ ������� ��� �� ����.
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (feof(file))
		{
			printf(
				"\n������: ���� �������� ������ �������, "
				"��� ��������� � ��� ��������� "
				"(%u �� %u)", i, orders.count
			);
			(void)_getch();
			exit(EXIT_FAILURE);
		}
		if (fscanf(
			file,
			"%s%s%s%ld%ld",
			orders.values[i].address,
			orders.values[i].kind,
			orders.values[i].date,
			&orders.values[i].weight,
			&orders.values[i].price) != 5)
		{
			printf(
				"\n������: ������ ����� %u ���������� ��� "
				"�������� � ������������ �������", i + 1
			);
			(void)_getch();
			exit(EXIT_FAILURE);
		}
		if (output)
		{
			fprintf(
				output,
				"%-20s %-20s %-11s %6ld %6ld\n",
				orders.values[i].address,
				orders.values[i].kind,
				orders.values[i].date,
				orders.values[i].weight,
				orders.values[i].price
			);
		}
		insert_order(&sp, orders.values[i]);
	}

	while (true) menu(orders, sp);
}
