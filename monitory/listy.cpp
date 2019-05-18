#include <stdio.h>
#include <forward_list>
#include <iostream>
#include <string.h>

using namespace std;

struct s
{
	int a;
	int b;
};

void set_s(s * ab, int a, int b)
{
	ab->a = a;
	ab->b = b;
}

int get_size(forward_list<s> lista)
{
	int counter = 0;
	for(forward_list<s>::iterator iter=lista.begin(); iter != lista.end(); iter++) counter++;

	return counter;
}

forward_list<s>::iterator find_next(forward_list<s> *lista)
{
	forward_list<s>::iterator i = lista->begin();
	int counter = 0;
	
	for(forward_list<s>::iterator iter=lista->begin(); iter != lista->end(); iter++)
	{
		if(iter->b == 1) counter++;
	}
	cout << counter << endl;

	if(counter == 0)
	{
		i = lista->before_begin();
		return i;
	}
	else counter = counter - 1;
		
	for(int j = 0; j < counter; j++) i++;
	return i;
}

int main()
{
	int i = 0;
	forward_list<s> lista;
   	s wiad;
	int liczba;



	liczba = get_size(lista);
	cout << liczba << endl;

	set_s(&wiad,0,0);

	while(i<5)
	{
		set_s(&wiad,i,0);
		lista.push_front(wiad);		
		i++;
	}
	
	liczba = get_size(lista);
	cout << liczba << endl;

	set_s(&wiad,1,1);
	i = 0;
	while(i<5)
	{
		liczba = lista.begin()->a;
		lista.pop_front();
		cout << liczba << endl;
		i++;
	}

	liczba = get_size(lista);
	cout << liczba << endl;

	forward_list<s>::iterator next = find_next(&lista);
	/*liczba = next->a;
	cout << liczba << endl;	*/


	/*for( int i = 0; i<3; i++)
	{
		set_s(&wiad,i+1,0);
		lista.insert_after(next,wiad);
		next++;
	}*/
	

	for(forward_list<s>::iterator iter=lista.begin(); iter != lista.end(); iter++)
	{
		liczba = iter->a;
		cout << liczba;
	}
			
	cout << endl << lista.max_size() << endl;

	return 0;
}
