// Include the precompiled headers
#include "pch.h"
#include "Game.h"

// Use some common namespaces to simplify the code
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;


// the class definition for the core "framework" of our app
// this is a view provider
ref class App sealed : public IFrameworkView
{
private:
	bool WindowClosed;
	CGame Game;
public:
	// some functions called by Windows
	// called when the app first starts up, gives us a chance to prepare the application
	virtual void Initialize(CoreApplicationView^ AppView)
	{
		// set the OnActivated function to handle to Activated "event"
		AppView->Activated += ref new TypedEventHandler
			<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

		// set suspending and resuming event handlers
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &App::Suspending);
		CoreApplication::Resuming += ref new EventHandler<Object^>(this, &App::Resuming);

		WindowClosed = false;
	}

	//called after initialize and allows us to set up things like window notications
	virtual void SetWindow(CoreWindow^ Window)
	{
		Window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::Closed);
	}

	//load things into memory
	virtual void Load(String^ EntryPoint) {}

	//most of the program will remain in this function. When the applicaiton is suspended, the code execution stops and then suspended is called.
	virtual void Run()
	{
		// init graphics stuff for directx
		Game.Initialize();

		CoreWindow ^window = CoreWindow::GetForCurrentThread();

		//if window is closed, end the loop
		while (!WindowClosed){
			//process events if they are present and then return
			window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			//update and render the game (the fun part!)
			Game.Update();
			Game.Render();
		}
	}

	virtual void Uninitialize() {}

	// an "event" that is called when the application window is ready to be activated
	void OnActivated(CoreApplicationView^ CoreAppView, IActivatedEventArgs^ Args)
	{
		CoreWindow ^Window = CoreWindow::GetForCurrentThread();
		Window->Activate();
	}

	void Suspending(Object ^Sender, SuspendingEventArgs ^Args) {}
	void Resuming(Object ^Sender, Object ^Args) {}

	// window is being closed
	void Closed(CoreWindow ^sender, CoreWindowEventArgs ^args)
	{
		WindowClosed = true;
	}
};

// the class definition that creates an instance of our core framework class
// this is a factory for view provider objects
ref class AppSource sealed : IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView()
	{
		return ref new App();    // create an App class and return it
	}
};

[MTAThread]    // define main() as a multi-threaded-apartment function

// the starting point of all programs
int main(Array<String^>^ args)
{
	CoreApplication::Run(ref new AppSource());    // create and run a new AppSource class
	return 0;
}