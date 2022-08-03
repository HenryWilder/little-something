#include "chem.hpp"

using namespace Element;
using namespace Molecule;

int main()
{
	auto dopamine = MakeMolecule(
		Carbon, Carbon, Carbon, Carbon, Carbon, Carbon, Carbon, Carbon, // C8
		Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, Hydrogen, // H11
		Nitrogen, // N
		Oxygen, Oxygen); // O2

	return 0;
}
