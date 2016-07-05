#include <Siv3D.hpp>
#include "CommandWIndow.hpp"

std::map<String, std::map<String, int>> ItemCommand::items;

void Main()
{
	FontAsset::Register(CommandFont, CommandFontSize);

	CommandWindow cwindow(Point(320, 0));

	cwindow().push_back(L"攻撃");
	cwindow().push_back(std::make_shared<ScrollCommand>(L"魔法", 12));
	cwindow().push_back(std::make_shared<ItemCommand>(L"アイテム", 6));

	for (const auto n : step(32))
		cwindow()(L"魔法").push_back(L"魔法" + Pad(n, { 2,L'0' }));

	for (const auto n : step(32))
	{
		const String name = L"アイテム" + Pad(n, { 2,L'0' });
		cwindow()(L"アイテム").push_back(std::make_shared<DecisionCommand>(name, L"使う", L"キャンセル"));
		ItemCommand::setItem(L"アイテム", name, n);
	}

	while (System::Update())
	{
		try
		{
			const String path = cwindow.update();
			Window::SetTitle(cwindow.getCursor());
			cwindow.draw();

			if (!path.isEmpty)
				Println(path);
		}
		catch (...)
		{

		}

	}
}
