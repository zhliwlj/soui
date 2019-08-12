// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
[!if CHECKBOX_SHELLNOTIFYICON]
#include "trayicon\SShellNotifyIcon.h"
[!endif]
//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE [!output ResLoaderType]
//#define SYSRES_TYPE [!output ResLoaderType]
// #define RES_TYPE 0   //PE
// #define RES_TYPE 1   //ZIP
// #define RES_TYPE 2   //7z
// #define RES_TYPE 2   //文件
//去掉多项支持，以免代码显得混乱
#if (RES_TYPE==1)
#include "resprovider-zip\zipresprovider-param.h"
#else 
#if (RES_TYPE==2)
#include "resprovider-7zip\zip7resprovider-param.h"
#endif
#endif
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif
[!if CHECKBOX_USE_LUA]
#ifdef _DEBUG
#pragma comment(lib,"lua-52d")
#pragma comment(lib,"scriptmodule-luad")
#else
#pragma comment(lib,"lua-52")
#pragma comment(lib,"scriptmodule-lua")
#endif
[!endif]


//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
ROBJ_IN_CPP

class SOUIEngine
{
private:
	SComMgr m_ComMgr;
	SApplication *m_theApp;
	bool m_bInitSucessed;
public:
	SOUIEngine(HINSTANCE hInstance):m_theApp(NULL), m_bInitSucessed(false){
		
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
		BOOL bLoaded = FALSE;
[!if RADIO_RANDER_GDI]
		//使用GDI渲染界面
		bLoaded = m_ComMgr.CreateRender_GDI((IObjRef * *)& pRenderFactory);
[!else]
		//使用SKIA渲染界面
		bLoaded = m_ComMgr.CreateRender_Skia((IObjRef * *)& pRenderFactory);
[!endif]
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		//设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		bLoaded = m_ComMgr.CreateImgDecoder((IObjRef * *)& pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		m_theApp = new SApplication(pRenderFactory, hInstance);	
		m_bInitSucessed = (TRUE==bLoaded);
	};
	operator bool()const
	{
		return m_bInitSucessed;
	}
	//加载系统资源
	bool LoadSystemRes()
	{
		BOOL bLoaded = FALSE;

[!if CHECKBOX_SYSRES_BUILTIN]
[!if CHECKBOX_RES_PACK_ONLAY_RELEASE]
#ifdef _DEBUG
			//选择了仅在Release版本打包资源则系统资源在DEBUG下始终使用DLL加载
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef * *)& sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				bLoaded=m_theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);				
			}
			else
			{
				SASSERT(0);
			}
		}
#else
		//钩选了复制系统资源选项
		{
			CAutoRefPtr<IResProvider> pSysResProvider;
[!if ResLoaderType == 0]
			CreateResProvider(RES_PE, (IObjRef**)& pSysResProvider);
			bLoaded = pSysResProvider->Init((WPARAM)m_theApp->GetInstance(), 0);
			SASSERT(bLoaded);
			bLoaded = m_theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef * *)& pSysResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]",_T("theme_sys_res"));
			bLoaded = pSysResProvider->Init((WPARAM)& param, 0);
			SASSERT(bLoaded);
			bLoaded = m_theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef * *)& pSysResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]", _T("theme_sys_res"));
			bLoaded = pSysResProvider->Init((WPARAM)& param, 0);
			SASSERT(bLoaded);
			bLoaded = m_theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			CreateResProvider(RES_FILE, (IObjRef**)& pSysResProvider);
			bLoaded = pSysResProvider->Init((LPARAM)_T("uires\\theme_sys_res"), 0);
			SASSERT(bLoaded);
			bLoaded = m_theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
		}
#endif
[!endif]
[!else]
		//从DLL加载系统资源
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef * *)& sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				m_theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
			}
		}

[!endif]
		return TRUE==bLoaded;
	}
	//加载用户资源
	bool LoadUserRes()
	{
		CAutoRefPtr<IResProvider>   pResProvider;
		BOOL bLoaded = FALSE;
[!if CHECKBOX_RES_PACK_ONLAY_RELEASE]
[!if ResLoaderType != 3]
#ifdef _DEBUG		
		//选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
		{
			CreateResProvider(RES_FILE, (IObjRef * *)& pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
		}
#else
[!endif]
		{
[!if ResLoaderType == 0]
			CreateResProvider(RES_PE, (IObjRef * *)& pResProvider);
			bLoaded = pResProvider->Init((WPARAM)m_theApp->GetInstance(), 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef * *)& pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)& param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef * *)& pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)& param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			CreateResProvider(RES_FILE, (IObjRef * *)& pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
[!endif]
		}
[!if ResLoaderType != 3]
#endif
[!endif]
[!endif]
		m_theApp->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID), TRUE);
		m_theApp->AddResProvider(pResProvider);
		return TRUE==bLoaded;
	}
	//加载LUA支持
	bool LoadLUAModule()
	{
		BOOL bLoaded =FALSE;
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = m_ComMgr.CreateScrpit_Lua((IObjRef * *)& pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		m_theApp->SetScriptFactory(pScriptLuaFactory);
		return TRUE == bLoaded;
	}
	//加载多语言支持
	bool LoadMultiLangsModule()
	{
		BOOL bLoaded = FALSE;
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = m_ComMgr.CreateTranslator((IObjRef * *)& trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			m_theApp->SetTranslator(trans);
			pugi::xml_document xmlLang;
			if (m_theApp->LoadXmlDocment(xmlLang,  _T("translator:lang_cn")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
		return TRUE == bLoaded;
	}
	//注册用户自定义皮肤和控件
	void Regitercustom()
	{
[!if CHECKBOX_SHELLNOTIFYICON]
		m_theApp->RegisterWindowClass<SShellNotifyIcon>();
[!endif]
	}

	~SOUIEngine()
	{
		if (m_theApp)
		{
			delete m_theApp;
			m_theApp = NULL;
		}
	}

	template<class MainWnd>
	int Run()
	{
		MainWnd dlgMain;
		dlgMain.Create(GetActiveWindow());
		dlgMain.SendMessage(WM_INITDIALOG);
		dlgMain.CenterWindow(dlgMain.m_hWnd);
[!if CHECKBOX_MAXIMIZED]
		dlgMain.ShowWindow(SW_SHOWMAXIMIZED);
[!else]
		dlgMain.ShowWindow(SW_SHOWNORMAL);
[!endif]
		return m_theApp->Run(dlgMain.m_hWnd);
	}

	SApplication* GetApp()
	{
		return m_theApp;
	}
};
//debug时方便调试设置当前目录以便从文件加载资源
void SetDefaultDir()
{
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(lpInsertPos + 1, _T("..\\[!output PROJECT_NAME]"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));
	SetDefaultDir();
	int nRet = 0;
	{
		SOUIEngine souiEngine(hInstance);
		if (souiEngine)
		{
			//加载系统资源
			souiEngine.LoadSystemRes();
			//加载用户资源
			souiEngine.LoadUserRes();
			//注册用户自定义皮肤/控件/布局/其它自定义
			souiEngine.Regitercustom();
[!if CHECKBOX_USE_LUA]
			//加载LUA脚本支持
			souiEngine.LoadLUAModule();
[!endif]
[!if CHECKBOX_TRANSLATOR_SUPPORT]
			//加载多语言翻译模块。
			souiEngine.LoadMultiLangsModule();
[!endif]
			nRet = souiEngine.Run<CMainDlg>();
		}
		else
		{
			MessageBox(NULL, _T("无法正常初使化SOUI"), _T("错误"), MB_OK | MB_ICONERROR);
		}
	}
	OleUninitialize();
	return nRet;
}
