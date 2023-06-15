#pragma once

// Windows API
#include <windows.h>
// Direct2D
#include <d2d1.h>
#pragma comment(lib, "d2d1")

template <class DERIVED_TYPE>
class BaseWindow {
public:

	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		DERIVED_TYPE* pointer = nullptr;
		
		if (msg == WM_NCCREATE) {
			CREATESTRUCT* createStructPointer = reinterpret_cast<CREATESTRUCT*>(lparam);
			pointer = (DERIVED_TYPE*) createStructPointer->lpCreateParams;

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pointer);
			
			pointer->setWindowHandle(hwnd);
		} else {
			pointer = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}

		if (pointer) {
			return pointer->handleMsg(msg, wparam, lparam);
		}
		// else
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	BaseWindow() : hwnd_(nullptr) {
	} 

	// this function is used to create the window
	// returns true if it succeded
	BOOL create(PCWSTR label, DWORD style = WS_OVERLAPPEDWINDOW, DWORD exStyle = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int width = CW_USEDEFAULT, int height = CW_USEDEFAULT, HWND parent = 0, HMENU menu = 0) {
		WNDCLASS windowClass = { 0 };
		windowClass.lpfnWndProc = DERIVED_TYPE::windowProc;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.lpszClassName = className();
		
		RegisterClass(&windowClass);

		hwnd_ = CreateWindowEx(exStyle, className(), label, style, x, y, width, height, parent, menu, GetModuleHandle(NULL), this);

		return (hwnd_ ? TRUE : FALSE);
	}
	
	HWND windowHandle() const {
		return hwnd_;
	}
protected:
	HWND hwnd_;

	// override this methode
	virtual PCWSTR className() const = 0;
	// override this methode
	virtual LRESULT handleMsg(UINT msg, WPARAM wparam, LPARAM lparam) = 0;
};

// used for releasing brushes and factories
template <class T> void safeRelease(T** ppT) {
	if (*ppT) {
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

// creates a window; helps with rendering
class RenderWindow : public BaseWindow<RenderWindow> {
public:
	RenderWindow() : pointerFactory(nullptr), pointerRenderTarget(nullptr), pointerBrush(nullptr), ellipse() {
	}

	// You can override this Methode
	void calculateLayout() {
		if (pointerRenderTarget != nullptr) {
			D2D1_SIZE_F size = pointerRenderTarget->GetSize();

			const float x = size.width / 2;
			const float y = size.height / 2;
			const float radius = min(x, y);
			
			ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
		}
	}

	HRESULT createGraphicsResources() {
		HRESULT result = S_OK;

		if (!pointerRenderTarget) { // == NULL
			RECT rect;
			GetClientRect(hwnd_, &rect);

			D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);

			result = pointerFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd_, size), &pointerRenderTarget);
			
			if (SUCCEEDED(result)) {
				const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0.0f);

				result = pointerRenderTarget->CreateSolidColorBrush(color, &pointerBrush);

				if (SUCCEEDED(result)) {
					calculateLayout();
				}
			}
		}
		
		return result;
	}

	void discardGraphicsResources() {
		safeRelease(&pointerRenderTarget);
		safeRelease(&pointerBrush);
	}

	// You can override this Methode
	void onPaint() {
		HRESULT result = createGraphicsResources();

		if (SUCCEEDED(result)) {
			PAINTSTRUCT paintStruct;
			BeginPaint(hwnd_, &paintStruct);

			pointerRenderTarget->BeginDraw();

			pointerRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF(0.1f, 0.1f, 0.1f))); // TODO BackgroundColor
			pointerRenderTarget->FillEllipse(ellipse, pointerBrush);

			result = pointerRenderTarget->EndDraw();
			if (FAILED(result) || result == D2DERR_RECREATE_TARGET) {
				discardGraphicsResources();
			}
			EndPaint(hwnd_, &paintStruct);
		}
	}

	void resize() {
		if (pointerRenderTarget) { // != NULL
			RECT rect;

			GetClientRect(hwnd_, &rect);

			D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);

			pointerRenderTarget->Resize(size);

			calculateLayout();

			InvalidateRect(hwnd_, nullptr, FALSE);
		}
	}

	// You have to override this Methode
	LRESULT handleMsg(UINT msg, WPARAM wparam, LPARAM lparam);

protected:
	ID2D1Factory* pointerFactory;
	ID2D1HwndRenderTarget* pointerRenderTarget;
	ID2D1SolidColorBrush* pointerBrush;

	// only used as an example
	D2D1_ELLIPSE ellipse;


	PCWSTR className() const {
		return L"render_window_class";
	}
};
