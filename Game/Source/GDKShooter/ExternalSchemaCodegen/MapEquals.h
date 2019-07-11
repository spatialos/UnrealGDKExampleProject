#pragma once

template<typename k, typename v>
bool operator==(TMap<k, v> Map1, TMap<k, v> Map2)
{
	if (Map1.Num() != Map2.Num())
	{
		return false;
	}
	for (const TPair<k, v>& Elem : Map1)
	{
		if (Elem.Value != Map2.FindRef(Elem.Key))
		{
			return false;
		}
	}
	return true;
}
