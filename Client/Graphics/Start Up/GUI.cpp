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
#include "SendFilePacket.h"
#include "RequestFilesPacket.h"
#include "RequestFileDownload.h"
int SelectedTab = 0;
int SelectedSubTab = 0;
int TabCount = 0;
int KeyBindClipBoard = 0;
EntityVector MenuEntity;
bool MenuOpen = true;
D2D1::ColorF ColourPickerClipBoard = D2D1::ColorF::Red;
std::wstring UsernameText = L"Username";
std::wstring PasswordText = L"Password";
std::wstring UploaderPath = L"Path";
void CreateDownloader();
void CreateGUI()
{
	MenuEntity = std::make_shared<Container>();
	auto form = std::make_shared<Form>(100, 100.0f, 300, 250, 2, 30, L"Enter", false);
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
					
						CreateDownloader();
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
void CreateDownloader()
{
	MenuEntity = std::make_shared<Container>();
	auto form = std::make_shared<Form>(100, 100.0f, 400, 350, 2, 30, L"File Manager", false);
	{
		auto tabcontroller = std::make_shared<TabController>();
		form->Push(tabcontroller);
		auto uploader = std::make_shared<Tab>(L"Uploader", 5, 60, &SelectedTab, 80, 20);
		{
			auto filepath = std::make_shared<TextBox>(10, 20, L"File Path", &UploaderPath);
			uploader->Push(filepath);
			auto fileselection = std::make_shared<Button>(180, 20, L"Select File", []()
				{
					HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
					if (!SUCCEEDED(hr))
						return;
					IFileOpenDialog* filedialog;
					hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&filedialog));
					if (!SUCCEEDED(hr))
						return;
					hr = filedialog->Show(NULL);

					if (!SUCCEEDED(hr))
						return;
					IShellItem* item;
					hr = filedialog->GetResult(&item);

					if (!SUCCEEDED(hr))
						return;
					PWSTR path;
					hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);

					if (!SUCCEEDED(hr))
						return;
					UploaderPath = path;
					CoTaskMemFree(path);

					item->Release();


					filedialog->Release();

					CoUninitialize();
					CreateDownloader();
				

				});
			uploader->Push(fileselection);
			auto fileupload = std::make_shared<Button>(10, 50, L"Upload File", []()
				{
					if (!std::filesystem::exists(UploaderPath))
						return;
					std::ifstream file(UploaderPath, std::ios::in | std::ios::binary);
					if (!file.is_open())
						return;
					std::vector<uint8_t> contents;
					file.unsetf(std::ios::skipws);
					contents.insert(
						contents.begin(),
						std::istream_iterator<uint8_t>(file),
						std::istream_iterator<uint8_t>()
					);
					file.close();
					std::filesystem::path path(UploaderPath);
					std::string filename = path.filename().string();
					SendFilePacket packet(filename);
					json jsoned;
					packet.ToJson(jsoned);
					TCPClient->SendText(jsoned.dump());
					TCPClient->SendData(contents);

				});
			uploader->Push(fileupload);
		}
		tabcontroller->Push(uploader);
		auto downloader = std::make_shared<Tab>(L"Downloader", 95, 60, &SelectedTab, 80, 20);
		{
			auto tablist = std::make_shared<TabListBoxController>(10, 40, 160, 240);
			{

				RequestFilesPacket packet;
				json jsoned;
				packet.ToJson(jsoned);
				TCPClient->SendText(jsoned.dump());
				printf(jsoned.dump().c_str());
				int amount = 0;
				while (true)
				{
					printf("Waiting for file list\n");
					std::string text = TCPClient->ReceiveText();
					if (text.size() == 0)
						continue;
					else
					{
						amount = std::stoi(text);
						break;
					}
				}
				if (amount != 0)
				{
					for (int i = 0; i < amount; i++)
					{
						std::string id = TCPClient->ReceiveText();
						int intid = std::stoi(id);
						std::string filename = TCPClient->ReceiveText();
						std::string filedate = TCPClient->ReceiveText();
						std::wstring wfilename(filename.begin(), filename.end());
						auto tablistbox = std::make_shared<TabListBox>(wfilename);
						std::wstring wfiledate(filedate.begin(), filedate.end());
						std::wstring wid(id.begin(), id.end());
						tablistbox->Push(std::make_shared<Label>( L"Filename :" + wfilename, 180, 0));
						tablistbox->Push(std::make_shared<Label>( L"Upload Date: "+wfiledate, 180, 20));
						tablistbox->Push(std::make_shared<Label>(L"ID:" + wid, 180, 40));
						tablistbox->Push(std::make_shared<Button>(180, 70, L"Download", []()
							{
							

							}));
						tablist->PushBack(tablistbox);

					}
				}
			}
			downloader->Push(tablist);
			auto getfiles = std::make_shared<Button>(10, 10, L"Refresh", []()
				{
					CreateDownloader();
				});
			downloader->Push(getfiles);

		}
		tabcontroller->Push(downloader);
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
