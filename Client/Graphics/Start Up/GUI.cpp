#include "pch.h"
#include "GUI.h"
#include "entity.h"
#include "Form.h"
#include "Button.h"
#include "ColourPicker.h"
#include "Label.h"
#include "tab.h"
#include "TabController.h"
#include "Toggle.h"
#include "Slider.h"
#include "DropDown.h"
#include "ComboBox.h"
#include "KeyBind.h"
#include "TabListBox.h"
#include "TabListBoxController.h"
#include "TextBox.h"
#include "Sockets.h"
#include "RegisterPacket.h"
#include "LoginPacket.h"
int SelectedTab = 0;
int SelectedSubTab = 0;
int TabCount = 0;
int KeyBindClipBoard = 0;
EntityVector MenuEntity;
bool MenuOpen = true;
D2D1::ColorF ColourPickerClipBoard = D2D1::ColorF::Red;
std::wstring UsernameText = L"Username";
std::wstring PasswordText = L"Password";

void CreateGUI()
{
	MenuEntity = std::make_shared<Container>();
	auto form = std::make_shared<Form>(100, 100.0f, 400, 350, 2, 30, L"FORM", false);
	{ 
		auto tabcontroller = std::make_shared<TabController>();
		form->Push(tabcontroller);
		auto loginpage = std::make_shared<Tab>(L"Login Page", 5, 60, &SelectedTab, 80, 20);
		{
			auto username = std::make_shared<TextBox>( 10, 20, L"Username", &UsernameText);
			auto password = std::make_shared<TextBox>( 10, 60, L"Password", &PasswordText);
			auto login = std::make_shared<Button>(10, 90, L"Login", []()
				{
					std::string username(UsernameText.begin(), UsernameText.end());
					std::string password(PasswordText.begin(), PasswordText.end());
					LoginPacket packet(username, password);
					json jsoned;
					packet.ToJson(jsoned);
					TCPClient->SendText(jsoned.dump());
					std::wstring response = L"";
					while (true)
					{
						std::string text = TCPClient->ReceiveText();
						if (text.size() == 0)
							continue;
						response = std::wstring(text.begin(), text.end());
						break;
					}
					MessageBox(NULL, response.c_str(), L"Login", MB_OK);
					if (response == L"Successful Login")
					{
					}
					else
					{
						CreateGUI();
					}
				
				});
			loginpage->Push(username);
			loginpage->Push(password);
			loginpage->Push(login);

		}
		auto registerpage = std::make_shared<Tab>(L"Register Page", 95, 60, &SelectedTab, 90, 20);
		{
			auto username = std::make_shared<TextBox>(10, 20, L"Username", &UsernameText);
			auto password = std::make_shared<TextBox>(10, 60, L"Password", &PasswordText);
			auto registeraction = std::make_shared<Button>(10, 90, L"Register", []()
				{
					std::string username(UsernameText.begin(), UsernameText.end());
					std::string password(PasswordText.begin(), PasswordText.end());
					RegisterPacket packet(username,password);
					json jsoned;
					packet.ToJson(jsoned);
					TCPClient->SendText(jsoned.dump());
					std::wstring response = L"";
					while (true)
					{
						std::string text = TCPClient->ReceiveText();
						if (text.size() == 0)
							continue;
						response = std::wstring(text.begin(),text.end());
						break;
					}
					MessageBox(NULL, response.c_str(), L"Register", MB_OK);
					CreateGUI();
				});
			registerpage->Push(username);
			registerpage->Push(password);
			registerpage->Push(registeraction);
		}
	tabcontroller->Push(loginpage);
	tabcontroller->Push(registerpage);
	}

	MenuEntity->Push(form);
	MenuEntity->Draw();
	MenuEntity->Update();
}

void SetFormPriority()
{
	// This sorts the host container (containerptr) which contains forms, as long as a form isn't parented to another form then this will allow it to draw over when clicked.
	// I swear to god if i need to make this work for forms inside forms for some odd reason in the future then i am going to throw a monitor out the window.
	std::sort(MenuEntity->GetContainer().begin(), MenuEntity->GetContainer().end(),
	          [](child a, child b) { return b->GetLastClick() < a->GetLastClick(); }
	);
}

float LastOpen = 0;

void Render()
{
	if (IsKeyClicked(VK_INSERT) && LastOpen < clock() * 0.00001f)
	{
		LastOpen = (clock() * 0.00001f) + 0.002f;
		MenuOpen = !MenuOpen;
	}

	MenuEntity->Draw();
	MenuEntity->GetContainer()[0]->Update(); // only allow stretching,dragging and other update stuff if it is the main form, prevents dragging and sizing the wrong forms.
	SetFormPriority();
}
