#pragma once

template <typename T>
T ReadPointer(DWORD ptr, DWORD offset)
{
	return (T)(ptr + offset);
}