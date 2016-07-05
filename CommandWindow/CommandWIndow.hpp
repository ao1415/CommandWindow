#pragma once

#include <Siv3d.hpp>
#include <deque>

#pragma warning(disable : 4290)

const int CommandFontSize = 12;
const String CommandFont = L"CommandFont";

class CommandWindow;

/// <summary>
/// コマンドウインドウのアイテム
/// </summary>
class Command {
public:

	/// <summary>コンストラクタ</summary>
	Command() : Command(L"") {};
	/// <summary>コンストラクタ</summary>
	/// <param name="str">コマンド名</param>
	Command(const String& str) : Command(Point(0, 0), str) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="str">コマンド名</param>
	Command(const Point& pos, const String& str) : Command(pos, Size(0, 0), str) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="size">追加する余白の大きさ</param>
	/// <param name="str">コマンド名</param>
	Command(const Point& pos, const Size& _size, const String& str) : size(_size), windowRect(pos, _size), string(str) {}

	/// <summary>指定されたコマンドを取り出します</summary>
	/// <param name="str">コマンド名</param>
	/// <returns>Commandの参照</returns>
	Command& operator()(const String& str) {
		return *nextCommandWindow[access.at(str)].get();
	}

	/// <summary>左上座標を取得します</summary>
	/// <returns>const Point</returns>
	const Point& getPos() const { return windowRect.pos; }
	/// <summary>ウインドウの大きさを取得します</summary>
	/// <returns>const Size</returns>
	const Size& getSize() const { return windowRect.size; }
	/// <summary>コマンド名を取得します</summary>
	/// <returns>const String</returns>
	const String& getString() const { return string; }

	/// <summary>次のウインドウの情報を取得します</summary>
	/// <returns>const Array<std::shared_ptr<Command>></returns>
	const Array<std::shared_ptr<Command>>& getWindow() const { return nextCommandWindow; }

	/// <summary>コマンドを追加します</summary>
	/// <param name="com">コマンド</param>
	/// <returns>*this</returns>
	Command& push_back(std::shared_ptr<Command>&& com) {
		access[com->string] = nextCommandWindow.size();

		maxStringNum = Max(maxStringNum, (int)com->string.length);
		const Rect rect = FontAsset(CommandFont)(com->string).region();
		windowRect.size.x = Max(windowRect.size.x, rect.size.x + 8 + size.x);
		windowRect.size.y += rect.size.y;

		nextCommandWindow.push_back(std::move(com));
		return *this;
	}

	/// <summary>コマンドを追加します</summary>
	/// <param name="str">コマンド名</param>
	/// <returns>*this</returns>
	Command& push_back(const String& str) {
		push_back(std::make_shared<Command>(str));
		return *this;
	}

	/// <summary>コマンドを追加します</summary>
	/// <param name="str">コマンド名</param>
	/// <returns>*this</returns>
	template<typename Type>
	Command& push_back(const String& str) {
		push_back(std::make_shared<Type>(str));
		return *this;
	}

	/// <summary>コマンドを追加します</summary>
	/// <param name="strs">String list</param>
	/// <returns>*this</returns>
	Command& push_back(const std::initializer_list<String>& strs) {
		for (const auto& str : strs)
			push_back(str);
		return *this;
	}

	/// <summary>コマンドを追加します</summary>
	/// <param name="strs">String list</param>
	/// <returns>*this</returns>
	template<typename Type>
	Command& push_back(const std::initializer_list<String>& strs) {
		for (const auto& str : strs)
			push_back<Type>(str);
		return *this;
	}

	//friend ItemCommand;
	friend CommandWindow;

protected:

	const bool inside(const int n) const { return (0 <= n && n < (int)nextCommandWindow.size()); }

	virtual const String update(std::deque<std::shared_ptr<Command>>& stack) {

		const int Size = (int)nextCommandWindow.size();

		if (Input::KeyZ.clicked)
		{
			if (nextCommandWindow[select]->nextCommandWindow.empty())
			{
				String str;
				for (const auto& s : stack)
					str += s->string + L"/";
				return str + nextCommandWindow[select]->string;
			}
			else
			{
				stack.push_back(nextCommandWindow[select]);
			}
		}
		else if (Input::KeyX.clicked)
		{
			stack.pop_back();
		}

		updateKeyUpDowm();

		return L"";
	}
	void updateKeyUpDowm() {

		const int Size = (int)nextCommandWindow.size();

		if (Input::KeyDown.clicked)
		{
			select = Min(select + 1, Size - 1);
		}
		else if (Input::KeyUp.clicked)
		{
			select = Max(select - 1, 0);
		}
		else if (Input::KeyDown.pressedDuration > 400 && keyCount % 3 == 0)
		{
			select = Min(select + 1, Size - 1);
			keyCount++;
		}
		else if (Input::KeyUp.pressedDuration > 400 && keyCount % 3 == 0)
		{
			select = Max(select - 1, 0);
			keyCount++;
		}
		else
			keyCount = 0;

	}

	virtual void draw(const Point& pos, const Color& ic, const Color& fc, const int inner) const {
		Rect(windowRect.pos + pos, windowRect.size).draw(ic).drawFrame(inner, 0, fc);

		const int FSize = CommandFontSize * 2;

		int n = 0;
		for (const auto& com : nextCommandWindow)
		{
			FontAsset(CommandFont).draw(com->string, windowRect.pos + pos + Point(4, FSize * n));
			n++;
		}

		Rect(windowRect.pos + pos + Point(4, 2 + FSize * select), windowRect.size.x - 8, 20).draw(Color(Palette::Lightblue, 127));

	}

	Size size;
	Rect windowRect;
	String string;

	int select = 0;
	int keyCount = 0;

	int maxStringNum = 0;

	Array<std::shared_ptr<Command>> nextCommandWindow;
	std::map<String, size_t> access;

};

const int ScrollBarWidth = 12;

/// <summary>
/// スクロールバーが付いたコマンドウインドウ
/// </summary>
/// <seealso cref="Command" />
class ScrollCommand : public Command {
public:

	/// <summary>コンストラクタ</summary>
	ScrollCommand(const int& _line) : ScrollCommand(L"", _line) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ScrollCommand(const String& str, const int& _line) : ScrollCommand(Point(0, 0), str, _line) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ScrollCommand(const Point& pos, const String& str, const int& _line) : ScrollCommand(pos, Size(ScrollBarWidth, 0), str, _line) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="size">ウインドウの大きさ</param>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ScrollCommand(const Point& pos, const Size& size, const String& str, const int& _line) : Command(pos, size, str), line(_line) {}

protected:

	const String update(std::deque<std::shared_ptr<Command>>& stack) override {
		const auto str = Command::update(stack);

		updateScroll();

		return str;
	}
	void updateScroll() {

		if (showLine > select)
			showLine = select;
		if (select > showLine + line - 1)
			showLine = select - line + 1;

	}

	virtual void draw(const Point& pos, const Color& ic, const Color& fc, const int inner) const override {

		const int FSize = CommandFontSize * 2;

		drawFrame(pos, ic, fc, inner);

		int n = 0;
		int sn = 0;
		for (const auto& com : nextCommandWindow)
		{
			if (showLine <= n && n < showLine + line)
			{
				FontAsset(CommandFont).draw(com->getString(), windowRect.pos + pos + Point(4, 0 + FSize * sn));
				sn++;
			}
			n++;
		}

		Rect(windowRect.pos + pos + Point(4, 2 + FSize * (select - showLine)), windowRect.size.x - 8 - ScrollBarWidth, 20).draw(Color(Palette::Lightblue, 127));

	}

	void drawFrame(const Point& pos, const Color& ic, const Color& fc, const int inner) const {

		const int FSize = CommandFontSize * 2;

		const Point bp = windowRect.pos + pos;

		Rect(bp, windowRect.size.x - ScrollBarWidth, FSize * line).draw(ic).drawFrame(inner, 0, fc);

		const Rect up(bp + Point(windowRect.size.x - ScrollBarWidth, 0), CommandFontSize);
		const Rect down(bp + Point(windowRect.size.x - ScrollBarWidth, FSize * line - CommandFontSize), CommandFontSize);
		const Rect bar(bp + Point(windowRect.size.x - ScrollBarWidth, CommandFontSize), Size(CommandFontSize, FSize * line - CommandFontSize - CommandFontSize));

		up.draw(ic).drawFrame(inner, 0, fc);
		down.draw(ic).drawFrame(inner, 0, fc);
		bar.draw(ic).drawFrame(inner, 0, fc);

		const Triangle upT(up.pos + Point(CommandFontSize / 2, inner + 1), up.pos + Point(inner + 1, CommandFontSize - inner - 1), up.pos + Point(CommandFontSize - inner - 1, CommandFontSize - inner - 1));
		const Triangle downT(down.pos + Point(CommandFontSize / 2, CommandFontSize - inner - 1), down.pos + Point(inner + 1, inner + 1), down.pos + Point(CommandFontSize - inner - 1, inner + 1));

		upT.draw();
		downT.draw();

		const int barSize = Min(bar.h*line / (int)nextCommandWindow.size(), bar.h);
		Rect(bar.pos + Point(0, bar.h*showLine / (int)nextCommandWindow.size()), Size(bar.w, barSize)).draw();

	}

	int line = 2;
	int showLine = 0;
};

/// <summary>
/// 個数が決まっているアイテムを扱うウインドウ
/// </summary>
/// <seealso cref="ScrollCommand" />
class ItemCommand : public ScrollCommand {
public:

	/// <summary>コンストラクタ</summary>
	ItemCommand(const int& _line) : ItemCommand(L"", _line) {};
	/// <summary>コンストラクタ</summary>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ItemCommand(const String& str, const int& _line) : ItemCommand(Point(0, 0), str, _line) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ItemCommand(const Point& pos, const String& str, const int& _line)
		: ItemCommand(pos, Size(12 + FontAsset(CommandFont)(L" 00").region().w, 0), str, _line) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="pos">左上の相対座標</param>
	/// <param name="size">ウインドウの大きさ</param>
	/// <param name="str">コマンド名</param>
	/// <param name="_line">コマンドの表示数</param>
	ItemCommand(const Point& pos, const Size& size, const String& str, const int& _line) : ScrollCommand(pos, size, str, _line) {}

	/// <summary>
	/// アイテムを設定・登録する
	/// </summary>
	/// <param name="tag">設定するタグ</param>
	/// <param name="item">登録するアイテム名</param>
	/// <param name="num">アイテムの個数</param>
	static void setItem(const String& tag, const String& item, const int num) { items[tag][item] = num; }

	/// <summary>
	/// アイテムの個数を取得する
	/// </summary>
	/// <param name="tag">登録されているタグ</param>
	/// <param name="item">アイテム名</param>
	/// <returns>アイテムの個数</returns>
	static int getItem(const String& tag, const String& item) { return items[tag][item]; }

	/// <summary>
	/// 指定したアイテムの個数を増やす
	/// </summary>
	/// <param name="tag">設定されているタグ</param>
	/// <param name="item">個数を増やすアイテム</param>
	/// <param name="num">増やす個数</param>
	static void itemInc(const String& tag, const String& item, const int num = 1) { items[tag][item] += num; }

	/// <summary>
	/// 指定したアイテムの個数を減らす
	/// </summary>
	/// <param name="tag">設定されているタグ</param>
	/// <param name="item">個数を減らすアイテム</param>
	/// <param name="num">減らす個数</param>
	static void itemDec(const String& tag, const String& item, const int num = 1) { items[tag][item] -= num; }

	/// <summary>登録されているすべてのアイテムを解放する</summary>
	static void reset() { items.clear(); }
	/// <summary>指定されたタグのアイテムを解放する</summary>
	/// <param name="tag">解放するタグ</param>
	static void reset(const String& tag) { items[tag].clear(); }

protected:

	const String update(std::deque<std::shared_ptr<Command>>& stack) override {

		const int Size = (int)nextCommandWindow.size();

		if (Input::KeyZ.clicked)
		{
			if (nextCommandWindow[select]->getWindow().empty())
			{
				String str;
				for (const auto& s : stack)
					str += s->getString() + L"/";
				return str + nextCommandWindow[select]->getString();
			}
			else
			{
				const String name = nextCommandWindow[select]->getString();
				const String tag = stack.back()->getString();

				if (items.at(tag).at(name) > 0)
					stack.push_back(nextCommandWindow[select]);
			}
		}
		else if (Input::KeyX.clicked)
		{
			stack.pop_back();
		}

		updateKeyUpDowm();

		updateScroll();

		return L"";
	}

	void draw(const Point& pos, const Color& ic, const Color& fc, const int inner) const override {
		ScrollCommand::drawFrame(pos, ic, fc, inner);

		const int FSize = CommandFontSize * 2;

		drawFrame(pos, ic, fc, inner);

		int n = 0;
		int sn = 0;
		for (const auto& com : nextCommandWindow)
		{
			if (showLine <= n && n < showLine + line)
			{
				String str = Format(com->getString()).padRight(maxStringNum, L'　') + L" "
					+ Pad(items.at(string).at(com->getString()), { 2,L'0' });
				FontAsset(CommandFont).draw(str, windowRect.pos + pos + Point(4, 0 + FSize * sn));
				sn++;
			}
			n++;
		}

		Rect(windowRect.pos + pos + Point(4, 2 + FSize * (select - showLine)), windowRect.size.x - 8 - ScrollBarWidth, 20).draw(Color(Palette::Lightblue, 127));

	}

private:

	static std::map<String, std::map<String, int>> items;

};

const String DecisionYesString = L"はい";
const String DecisionNoString = L"いいえ";

/// <summary>
/// コマンド決定ウインドウ
/// </summary>
/// <seealso cref="Command" />
class DecisionCommand : public Command {
public:

	DecisionCommand(const String& str) : DecisionCommand(Point(0, 0), Size(0, 0), str) {}
	DecisionCommand(const String& str, const String& yes, const String& no) : DecisionCommand(Point(0, 0), Size(0, 0), str, yes, no) {}
	DecisionCommand(const Point& pos, const String& str) : DecisionCommand(pos, Size(0, 0), str) {}
	DecisionCommand(const Point& pos, const Size& size, const String& str) : DecisionCommand(pos, size, str, L"はい", L"いいえ") {}
	DecisionCommand(const Point& pos, const Size& size, const String& str, const String& yes, const String& no) : Command(pos, size, str) {
		push_back({ yes,no });
		select = 1;
	}

protected:

	virtual const String update(std::deque<std::shared_ptr<Command>>& stack) override {

		const int Size = (int)nextCommandWindow.size();

		if (Input::KeyZ.clicked)
		{
			if (nextCommandWindow[select]->getWindow().empty())
			{
				if (select == 0)
				{
					String str;
					for (const auto& s : stack)
						str += s->getString() + L"/";
					str += nextCommandWindow[select]->getString();
					select = 1;
					return str;
				}
				else
				{
					stack.pop_back();
					select = 1;
				}
			}
			else
			{
				stack.push_back(nextCommandWindow[select]);
			}
		}
		else if (Input::KeyX.clicked)
		{
			stack.pop_back();
			select = 1;
		}

		updateKeyUpDowm();

		return L"";
	}

private:


};

/// <summary>
/// アイテムを選択できるウインドウ
/// </summary>
class CommandWindow {
public:

	/// <summary>コンストラクタ</summary>
	CommandWindow() : CommandWindow(Point(0, 0), Size(0, 0)) {};
	/// <summary>コンストラクタ</summary>
	/// <param name="_pos">ウインドウの左上座標</param>
	CommandWindow(const Point& _pos) : CommandWindow(_pos, Size(0, 0)) {}
	/// <summary>コンストラクタ</summary>
	/// <param name="_pos">ウインドウの左上座標</param>
	/// <param name="_size">ウインドウの大きさ</param>
	CommandWindow(const Point& _pos, const Size& _size) : pos(_pos) {
		baseCommandWindow = std::make_unique<Command>(Point(0, 0), _size, L"base");
		windowStack.push_back(baseCommandWindow);
	}

	/// <summary>最上層のコマンドを返します</summary>
	/// <returns>*this</returns>
	Command& operator()() {
		return *baseCommandWindow.get();
	}

	/// <summary>
	/// <para>コマンドの選択した文字列を返します</para>
	/// <para>　</para>
	/// <para>例外</para>
	/// <para>std::out_of_range</para>
	/// </summary>
	/// <returns>String</returns>
	const String update() throw(std::out_of_range) {
		if (windowStack.empty())
		{
			windowStack.push_front(baseCommandWindow);
			throw std::out_of_range("コマンドがキャンセルされました");
		}
		else
		{
			auto& p = windowStack.back();
			const String str = p->update(windowStack);
			if (!str.isEmpty)
			{
				windowStack.clear();
				windowStack.push_front(baseCommandWindow);
			}
			return str;
		}
	}

	/// <summary>ウインドウを描きます</summary>
	/// <param name="ic">内側の色</param>
	/// <param name="fc">枠の色</param>
	/// <param name="inner">枠の太さ</param>
	void draw(const Color& ic = Palette::Black, const Color& fc = Palette::White, const int inner = 1) const {
		auto deque = windowStack;
		Point p = pos;
		for (const auto& d : windowStack)
		{
			d->draw(p, ic, fc, inner);
			p.x += d->getSize().x;
		}
	}

	const String getCursor() const {
		String str;

		if (!windowStack.empty())
		{
			for (const auto& s : windowStack)
				str += s->getString() + L"/";
			str += windowStack.back()->nextCommandWindow[windowStack.back()->select]->getString();
		}
		return str;
	}

private:

	std::shared_ptr<Command> baseCommandWindow;
	std::deque<std::shared_ptr<Command>> windowStack;

	Point pos;

};

template <class CharType>
inline std::basic_ostream<CharType>& operator << (std::basic_ostream<CharType>& os, const Command& r)
{
	return os << CharType('(') << r.getPos() << CharType(',') << r.getSize() << CharType(')');
}
