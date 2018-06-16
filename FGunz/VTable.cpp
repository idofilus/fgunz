#include "VTable.h"
#include "detours.h"

using namespace Utils;

VtableBase::VtableBase(void)
{
	memset(this, NULL, sizeof(VtableBase));
}
VtableBase::VtableBase(VTABLE* instance)
{
	this->m_instance = instance;
	this->m_vtable = *instance;
	this->m_size = GetVtableSize(this->m_vtable);
}
VFUNC VtableBase::GetIndex(int index)
{
	return (this->m_instance == NULL) ? NULL : this->m_vtable[index];
}

VtableHook::VtableHook(VTABLE* instance) : VtableBase(instance)
{
	this->m_hooked = new VFUNC[this->m_size];

	memcpy(this->m_hooked, this->m_vtable, sizeof(VFUNC) * this->m_size);
	*this->m_instance = this->m_hooked;
}
VtableHook::~VtableHook(void)
{
	if (this->m_instance == NULL || this->m_vtable == NULL)
		return;

	*this->m_instance = this->m_vtable;
}
bool VtableHook::HookIndex(int index, VFUNC function)
{
	if (IsInitialized() == false || index >= this->m_size)
		return false;

	this->m_hooked[index] = function;
	return true;
}
bool VtableHook::UnhookIndex(int index)
{
	if (IsInitialized() == false || index >= this->m_size)
		return false;

	this->m_hooked[index] = this->m_vtable[index];
	return true;
}
VFUNC VtableHook::GetOriginal(int index)
{
	if (IsInitialized() == false || index >= this->m_size)
		return NULL;

	return this->m_vtable[index];
}

VtableDetour::VtableDetour(VTABLE* instance) : VtableBase(instance)
{
	this->m_hooked = new VDETOUR[this->m_size];
	memset(this->m_hooked, NULL, sizeof(VDETOUR) * this->m_size);
}
VtableDetour::~VtableDetour(void)
{
	// Remove all our Detours..
	for (int i = 0; i < this->m_size; i++)
		this->UnhookIndex(i);
}
bool VtableDetour::HookIndex(int index, VFUNC function)
{
	if (IsInitialized() == false || index >= this->m_size)
		return false;

	VDETOUR* Detour = &this->m_hooked[index];

	Detour->m_pFunction = function;
	//Detour->m_pTrampoline = (VFUNC)DetourFunction((PBYTE)this->m_vtable[index], (PBYTE)function);
	return true;
}
bool VtableDetour::UnhookIndex(int index)
{
	if (IsInitialized() == false || index >= this->m_size || this->m_hooked[index].m_pTrampoline == NULL)
		return false;

	VDETOUR* Detour = &this->m_hooked[index];

	//DetourRemove((PBYTE)Detour->m_pTrampoline, (PBYTE)Detour->m_pFunction);
	Detour->m_pFunction = NULL;
	Detour->m_pTrampoline = NULL;
	return true;
}
VFUNC VtableDetour::GetTrampoline(int index)
{
	if (index >= this->m_size)
		return NULL;

	return this->m_hooked[index].m_pTrampoline;
}