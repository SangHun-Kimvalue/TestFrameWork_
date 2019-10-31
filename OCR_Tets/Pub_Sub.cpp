#include <iostream>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include <Windows.h>

using namespace std;

class EventArgs {
public:
	virtual ~EventArgs() {}
};

class StringEventArgs : public EventArgs {
	string payload_;
public:
	explicit StringEventArgs(const string& payload) : payload_(payload) {}
	const string& Payload() const { return payload_; }
};

class Event {
	class Callback {
		void* pSender_;
		const EventArgs& args_;

	public:
		Callback(void* pSender, const EventArgs& args) : pSender_(pSender), args_(args) {}
		void operator()(pair<long, function<void(void*, const EventArgs&)>> p) const {
			p.second(pSender_, args_);
		}
	};
	map<long, function<void(void*, const EventArgs&)>> callbacks_;
	long token_ = 0;

public:
	void operator()(void* pSender, const EventArgs& args) const {
		for_each(callbacks_.begin(), callbacks_.end(), Callback(pSender, args));
	}
	long Subscribe(function<void(void*, const EventArgs&)> f) {
		token_++;
		callbacks_.insert(make_pair(token_, f));
		return token_;
	}
	void Unsubscribe(long token) {
		callbacks_.erase(token);
	}
};

class Publisher {
	Event event_;
	string name_;

public:
	explicit Publisher(const string& name) : name_(name) {}
	const string& Name() const { return name_; }
	void Publish(const string& message) {
		event_(this, StringEventArgs(message));
	}
	long Register(function<void(void*, const EventArgs&)> f) {
		return event_.Subscribe(f);
	}
	void Unregister(long token) {
		event_.Unsubscribe(token);
	}
};

class Subscriber {
	string name_;

public:
	explicit Subscriber(const string& name) : name_(name) {}
	void OnEventReceived(void* pSender, const EventArgs& args) {
		const StringEventArgs* const s = dynamic_cast<const StringEventArgs* const>(&args);
		if (s == nullptr)
			return;
		if (pSender == nullptr)
			return;
		Publisher* p = static_cast<Publisher*>(pSender);
		cout << name_.c_str() << " has received " << s->Payload().c_str() << " from " << p->Name().c_str() << endl;
	}
};

namespace {
	using namespace std::placeholders;
	long Subscribe(Publisher& publisher, Subscriber& subscriber) {
		return publisher.Register(bind(&Subscriber::OnEventReceived, &subscriber, _1, _2));
	}
	void Unsubscribe(Publisher& publisher, long token) {
		publisher.Unregister(token);
	}
}

class Subscription {
	Publisher& publisher_;
	long token_;

public:
	Subscription(Publisher& publisher, long token) : publisher_(publisher), token_(token) {
	}
	~Subscription() {
		publisher_.Unregister(token_);
	}
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	AllocConsole();

	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	Publisher prod("Publisher");
	Subscriber sub_1("Subscriber-1");
	Subscriber sub_2("Subscriber-2");

	long token_1 = Subscribe(prod, sub_1);
	long token_2 = Subscribe(prod, sub_2);

	prod.Publish("42");
	Unsubscribe(prod, token_1);
	prod.Publish("43");
	Unsubscribe(prod, token_2);
	prod.Publish("44");


	fclose(cp);
	FreeConsole();

	return 0;
}