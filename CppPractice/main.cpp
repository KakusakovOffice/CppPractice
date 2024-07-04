#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>

#include <conio.h>

using namespace std;
using namespace System;
using namespace System::IO;

// Структура описывающая один заказ еды.
struct order
{
	char address[20];  // Адрес заказа.
	char kind[20];  // Тип продукта.
	char date[11];  // Дата поступления заказа.
	long weight;  // Вес заказа.
	long price;  // Цена заказа.
};

// Двусторонний связный список, хранящий 
// данные о суммарной цене и кол-ве 
// заказов определенного типа.
struct list
{
	char kind[20];  // Тип продукта
	long count;  // Количество заказов с продуктом.
	long price;  // Суммарная цена заказов.
	struct list* next;  // Следующий узел списка.
	struct list* prev;  // Предыдущий узел списказ.
};

// Динамический массив данных типа struct order.
struct orders
{
	unsigned int count;  // Количество заказов еды.
	struct order* values;  // Массив заказов еды.
};

// Описание:
// Печатает в консоль (переписывает весь экран)
// данные массива orders.
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

// Описание:
// Печатает в консоль (после курсора)
// заказ с наибольшей ценой.
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
	printf("Самый дорогой заказ: %ld руб.\n", best->price);
	Console::CursorLeft = 10;
	printf("Его адрес: %s\n", best->address);
	(void)_getch();
}

// Описание:
// Переводит дату из формата 'гггг_мм_дд' (from)
// в формат 'дд месяц гггг' (to).
// 
// Если участки памяти to и from пересекаются, 
// то поведение функции неопределено.
// 
// Аргументы:
// to   -> Указатель на массив (24+ симв.), 
//         в котором будет сохранена дата в 
//         итоговом формате как 
//         нуль-терминированная строка.
// from -> Указатель на нуль-терминированную 
//         строку (11 симв. включая нуль),
//         в которой содержится дата в 
//         изначальном формате.
static void to_pretty_date(char* to, const char* from)
{
	char tmp[3];
	const char* months[] = {
		"января","февраля","марта","апреля","мая","июня",
		"июля","августа","сентября","октября","ноября","декабря"
	};
	strcpy(to, from + 8);
	strcat(to, " ");
	strncpy(tmp, from + 5, 2);
	tmp[2] = '\0';
	strcat(to, months[atoi(tmp) - 1]);
	strcat(to, " ");
	strncat(to, from, 4);
}

// Описание:
// Печатает в консоль (после курсора)
// заказ с самой поздней датой.
//
// Если заказов с такой датой несколько, 
// то печатает любой из них.
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
	printf("Последний заказ:\n");
	Console::CursorLeft = 12;
	printf("открыт: %s\n", pretty_date);
	Console::CursorLeft = 12;
	printf("адрес: % s\n", best->address);
	Console::CursorLeft = 12;
	printf("тип: %s\n", best->kind);
	Console::CursorLeft = 12;
	printf("сумма: %ld\n", best->price);
	(void)_getch();
}

// Описание:
// Печатает в консоль (после курсора)
// количество заказов пиццы.
static void print_pizza_order_count(struct orders orders)
{
	unsigned int total = 0;
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (strcmp(orders.values[i].kind, "пицца") == 0) total++;
	}

	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::BackgroundColor = ConsoleColor::Black;

	Console::CursorLeft = 10;
	printf("\nКоличество заказов пиццы: %u", total);
	(void)_getch();
}

// Описание:
// Выдкляет память и записывает новый узел списка,
// с пустыми указателями на следующий и предыдущий
// узлы.
//
// Аргументы:
// order -> Единственный заказ который будет
//          описываться узлом.
// 
// Возвращает:
// Новый узел списка. Чтобы освободить память
// можно использовать free().
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

// Описание:
// Добавляет заказ в список sp сортируя его по типу 
// заказа, при надобности выделяя память.
//
// Функция гарантирует, что sp всегда будет 
// указывать на начало списка.
//
// Если порядок сортировки списка sp был нарушен,
// то поведение функции неопределено.
//
// Аргументы:
// order -> Заказ, который будет добавлен в список sp.
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

// Описание:
// Печатает в консоль (переписывает весь экран)
// данные списка sp в алфавитном порядке 
// (сортировка идет по типу заказа).
static void list_alpha(struct list* sp)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("Алфавитный список по типу заказа.");
	printf("\n\r----------------------------------------------------------------");
	for (struct list* nt = sp; nt != NULL; nt = nt->next)
	{
		printf(
			"\n\r%-20s %7ld шт. %7ld р.",
			nt->kind,
			nt->count,
			nt->price
		);
	}
	(void)_getch();
}

// Описание:
// Печатает в консоль (переписывает весь экран)
// данные списка sp в обратном алфавитном порядке
// (сортировка идет по типу заказа).
static void list_alpha_reverse(struct list* sp)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("Обратный алфавитный список по типу заказа.");
	printf("\n\r----------------------------------------------------------------------------");
	struct list* tail = sp;
	while (tail->next != NULL) tail = tail->next;
	for (struct list* nt = tail; nt != NULL; nt = nt->prev)
	{
		printf(
			"\n\r%-20s %7ld шт. %7ld р.",
			nt->kind,
			nt->count,
			nt->price
		);
	}
	(void)_getch();
}

// Описание:
// Печатает в консоль (переписывает весь экран)
// список заказов из массива orders, цена 
// которых превышает 1000.
static void print_expensive_list(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("\n");
	Console::CursorLeft = 12;
	printf("Список заказов на сумму более 1.000 руб");
	printf("\n\r----------------------------------------------------------------");
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (orders.values[i].price > 1000)
		{
			printf(
				"\n\r%-20s %-20s %7ld г %7ld р.",
				orders.values[i].address,
				orders.values[i].kind,
				orders.values[i].weight,
				orders.values[i].price
			);
		}
	}
	(void)_getch();
}

// Описание:
// Печатает в консоль (переписывает весь экран)
// диаграмму по наиболее дорогим заказам,
// основанную на данных списка sp.
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
		printf("%20s %7ld шт. %20s", nt->kind, nt->count, str);
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

// Описание:
// Печатает в консоль (после курсора)
// первое совпадение цен заказа для
// двух заказов из массива orders с 
// разными типами.
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
	printf("Первый продукт с совпадающей ценой:\n");
	Console::CursorLeft = 10;
	if (found_match)
	{
		printf("цена: %7ld\n", a.price);
		Console::CursorLeft = 10;
		printf("тип 1: %-20s\n", a.kind);
		Console::CursorLeft = 10;
		printf("тип 2: %-20s\n", b.kind);
	}
	else printf("Совпадений не найдено.\n");
	(void)_getch();
}

// Описание:
// Печатает в консоль (переписывает весь экран)
// все совпадения цен заказа для
// двух заказов из массива orders с 
// разными типами.
static void match(struct orders orders)
{
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Gray;
	Console::Clear();
	printf("Список продуктов с совпадающей ценой и разным типом:");
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

// Описание:
// Отрисовывает меню и позволяет выбрать 
// одну из функций приложения при помощи
// клавиатуры.
static void menu(struct orders orders, struct list* sp)
{
	const char* options[] = {
		"                                      ",
		"Адрес самого дорогого заказа          ",
		"Последний заказ                       ",
		"Алфавитный список по типу             ",
		"Алфавитный список по типу (обратный)  ",
		"Заказы на сумму свыше 1.000 р.        ",
		"Количество заказов пиццы              ",
		"Диаграмма                             ",
		"Продукты с совпадающей ценой          ",
		"Продукты с совпадающей ценой (первый) ",
		"Исходные данные                       ",
		"Выход                                 ",
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
			case 11: exit(0);
			}
		case esc:    exit(0);
		}

		if (y > last_option) y = first_option;
		if (y < first_option) y = last_option;
	}
}

// Функция-помошник, чтобы удобно создавать файл с данными.
static void make_file(const char* path)
{
	FILE* file = fopen(path, "w");
	if (fprintf(
		file,
		"15\n"
		"ул_Ленина_д10 яблоки        2024-11-12 5000 2000\n"
		"ул_Сталина_д3 пицца         2024-11-11 500  2500\n"
		"ул_Пушкина_д3 суповой_набор 2024-11-12 250  500\n"
		"ул_Пушкина_д2 мороженное    2024-11-12 100  400\n"
		"д_Колотушкина пицца         2024-11-11 950  6500\n"
		"ул_Ввод_д6    суши          2024-11-10 500  2500\n"
		"ул_Кукина_д3  шоколад       2024-11-11 350  2000\n"

		"Кремль        шоколад       2024-12-11 1000 10000\n"

		"ул_Ленина_д12 капуста       2024-11-12 2250 250\n"
		"ул_Сталина_д7 пицца         2024-11-11 500  2500\n"
		"ул_Пушкина_д4 сыр_коровий   2024-11-12 250  500\n"
		"ул_Пушкина_д7 мороженное    2024-11-12 100  400\n"
		"ул_Пушкина_д9 суповой_набор 2024-11-11 100  400\n"
		"ул_Ввод_д8    шоколад       2024-11-13 250  1250\n"
		"ул_Кукина_д5  суши          2024-11-11 700  5000\n"
	) < 0) exit(1);
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
			"\nошибка: Не удается открыть файл %s "
			"для чтения: %s", path, strerror(errno)
		);
		(void)_getch();
		exit(1);
	}

	struct orders orders = {0};

	if (fscanf(file, "%u", &orders.count) != 1)
	{
		printf(
			"\nошибка: файл должен начинаться "
			"количеством записей (положительное число)"
		);
		(void)_getch();
		exit(1);
	}

	if (orders.count == 0)
	{
		printf("\nошибка: количество записей не может быть нулевым");
		(void)_getch();
		exit(1);
	}

	orders.values = (struct order*)malloc(sizeof(struct order) * orders.count);
	if (orders.values == NULL) abort();
	struct list* sp = NULL;  // Список заказов еды по типу.
	for (unsigned int i = 0; i < orders.count; i++)
	{
		if (feof(file))
		{
			printf(
				"\nошибка: файл содержит меньше записей, "
				"чем объявлено в его заголовке "
				"(%u из %u)", i, orders.count
			);
			(void)_getch();
			exit(1);
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
				"\nошибка: запись номер %u повреждена или "
				"записана в некорректном формате", i + 1
			);
			(void)_getch();
			exit(1);
		}
		insert_order(&sp, orders.values[i]);
	}

	while (1) menu(orders, sp);
}
