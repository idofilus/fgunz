#pragma once
#include <Windows.h>

// https://www.unknowncheats.me/forum/c-and-c-/70202-virtual-table-hook-classes.html

typedef PROC* VTABLE;
typedef PROC VFUNC;

namespace Utils
{
	inline int GetVtableSize(VTABLE vtable)
	{
		int i = 0;
		while (vtable[i] && IsBadCodePtr(reinterpret_cast<FARPROC>(vtable[i])) == FALSE)
			++i;

		return i;
	}

	class VtableBase
	{
	public:
		VtableBase(void);
		VtableBase(VTABLE* instance);

		virtual VFUNC GetIndex(int index);
		virtual bool HookIndex(int index, VFUNC func) = 0;
		virtual bool UnhookIndex(int index) = 0;

	protected:

		VTABLE * m_instance;
		VTABLE		m_vtable;
		int			m_size;
	};

	class VtableHook : public VtableBase
	{
	public:
		VtableHook(VTABLE* instance);
		~VtableHook(void);

		virtual bool HookIndex(int index, VFUNC func);
		virtual bool UnhookIndex(int index);
		virtual VFUNC GetOriginal(int index);

	protected:

		inline bool IsInitialized(void)
		{
			return (*this->m_vtable != NULL);
		};

		VTABLE		m_hooked;
	};

	class VtableDetour : public VtableBase
	{
	public:

		VtableDetour(VTABLE* instance);
		~VtableDetour(void);

		virtual bool HookIndex(int index, VFUNC function);
		virtual bool UnhookIndex(int index);
		virtual VFUNC GetTrampoline(int index);

	protected:

		struct VDETOUR
		{
			VFUNC		m_pTrampoline;
			VFUNC		m_pFunction;
		};

		inline BOOL IsInitialized(void)
		{
			return (*this->m_vtable != NULL);
		};

		VDETOUR		*m_hooked;
	};
}