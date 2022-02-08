#include <uwebsockets/App.h>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

// структура для хранения данных юзера при подключении
struct ClientDataConnection
{
	std::int32_t userID;
	std::string *userName;

};

int port = 40400;
std::string messageOnStartServer = "Сервер стартовал успешно!";
// ID последний юзера
std::atomic_ulong lastUserID = 6;
// создание потоков, флаг hardware_concurrency - означ. сколько потоков подде. машина данная
std::vector<std::thread*> threads(std::thread::hardware_concurrency());

int main()
{
	setlocale(LC_ALL, "Russian");
	// пробегаем по колекции тредов с помощью итератора
	transform(threads.begin(), threads.end(), threads.begin(), [&](auto* thr)
		{
			// лямда функиця для выполнение действий над тредами, при этом мы создаем новый поток
			return  new std::thread([&]()
				{
					// создаю APP  с lib с переменными через функцию ws
					// "/*" адресс вебсервера
					uWS::App().ws<ClientDataConnection>("/*",
						{
							// функция при открытие коннекта, лямбда функция с использ. переменной auto и указателя
							// обращение через & означает обращение к любым данным по ссылке
							.open = [&](auto* ws)
						{

							// фун-я для хранения данных, указатель для анонимных данных (sizeof(ClientDataConnection))
							// привидение к формату пользова. данных
							ClientDataConnection* data = (ClientDataConnection*)ws->getUserData();
							data->userID = lastUserID++;
							std::cout << "Новый юзер пришел " << data->userID << " " << std::endl;

							// подпись и получение сообщений от юзеров
							// функция alertmsg о подключении нового юзера всем
							ws->subscribe("Оповещение");

						},
						//функция при подключении юзера
						// функция обмена сообщениями 
					.message = [](auto* ws, std::string_view message, uWS::OpCode opcode)
					{
							ClientDataConnection* data = (ClientDataConnection*)ws->getUserData();
							std::cout << "Новое сообщение = " << message << " от юзера  " << data->userName << std::endl;
							auto startMesg = message.substr(0, 9);
							if (startMesg.compare("SET_NAME") == NULL) 
							{
								//Юзер прислал свое имя
								auto name = message.substr(9);
								data->userName = new std::string(name);
								//std::string* strName = new std::string(name);
								//data->userName = strName;
								std::cout << "Юзер с ID " << data->userID << " И Именем " << (*data->userName) << " " << std::endl;
								std::string broadCast = "NEW_USER" + (*data->userName);
								ws->publish("Оповещение", broadCast, opcode, true);
							}

					}
							// порт для прослушки
						}).listen(port, [](auto* tokenServer) 
							{
								// проверка на валид
								if (tokenServer)
								{
									std::cout << messageOnStartServer << " на порту " << port << std::endl;
								}
								else
								{
									std::cout << "Server failed on start!" << std::endl;
								}
							}).run();
				});
		});
	// обраба. все треды и джойним 
	for_each(threads.begin(), threads.end(), [](auto* thr) {thr->join(); });



}


