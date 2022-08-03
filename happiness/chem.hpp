#pragma once

#define _countof(x) (sizeof(x) / sizeof(x[0]))

template<typename T, int SIZE>
struct ConstArray
{
	template<typename ...Pack>
	consteval ConstArray(Pack&&..._Val) : data{ _Val... } {}

	const T data[SIZE];
	constexpr T operator[](int i) const { return data[i]; }
	constexpr int size() const { return SIZE; }
	constexpr T* begin() const { return data; }
	constexpr T* end() const { return data + SIZE; }
};

template<int SIZE> using ConstIntArray = ConstArray<int, SIZE>;
template<int SIZE> using ConstStringArray = ConstArray<const char*, SIZE>;

namespace Element
{
	using AMU_t = double;
	using KG_t = double;

	constexpr KG_t AmuToKg(AMU_t amu) { return amu * 1.67377e-27; }
	constexpr AMU_t KgToAmu(KG_t kg) { return kg * 6.0229552894949e+26; }
	constexpr AMU_t g_protonMass = 1.00727647;
	constexpr AMU_t g_neutronMass = 1.008665;
	constexpr AMU_t g_electronMass = 0.000548579909;

	enum class Group
	{
		AlkaliMetal,
		AlkalineEarthMetal,
		TransitionMetal,
		PostTransitionMetal,
		Metaloid,
		ReactiveNonMetal,
		NobleGas,
		Lanthanide,
		Actinide,
		Unknown,
	};

	constexpr ConstStringArray<109> names
	{
		"Hydrogen",
		"Helium",
		"Lithium",
		"Beryllium",
		"Boron",
		"Carbon",
		"Nitrogen",
		"Oxygen",
		"Fluorine",
		"Neon",
		"Sodium",
		"Magnesium",
		"Aluminum",
		"Silicon",
		"Phosphorus",
		"Sulfur",
		"Chlorine",
		"Argon",
		"Potassium",
		"Calcium",
		"Scandium",
		"Titanium",
		"Vanadium",
		"Chromium",
		"Manganese",
		"Iron",
		"Cobalt",
		"Nickel",
		"Copper",
		"Zinc",
		"Gallium",
		"Germanium",
		"Arsenic",
		"Selenium",
		"Bromine",
		"Krypton",
		"Rubidium",
		"Strontium",
		"Yttrium",
		"Zirconium",
		"Niobium",
		"Molybdenum",
		"Technetium",
		"Ruthenium",
		"Rhodium",
		"Palladium",
		"Silver",
		"Cadmium",
		"Indium",
		"Tin",
		"Antimony",
		"Tellurium",
		"Iodine",
		"Xenon",
		"Cesium",
		"Barium",
		"Lanthanum",
		"Cerium",
		"Praseodymium",
		"Neodymium",
		"Promethium",
		"Samarium",
		"Europium",
		"Gadolinium",
		"Terbium",
		"Dysprosium",
		"Holmium",
		"Erbium",
		"Thulium",
		"Ytterbium",
		"Lutetium",
		"Hafnium",
		"Tantalum",
		"Tungsten",
		"Rhenium",
		"Osmium",
		"Iridium",
		"Platinum",
		"Gold",
		"Mercury",
		"Thallium",
		"Lead",
		"Bismuth",
		"Polonium",
		"Astatine",
		"Radon",
		"Francium",
		"Radium",
		"Actinium",
		"Thorium",
		"Protactinium",
		"Uranium",
		"Neptunium",
		"Plutonium",
		"Americium",
		"Curium",
		"Berkelium",
		"Californium",
		"Einsteinium",
		"Fermium",
		"Mendelevium",
		"Nobelium",
		"Lawrencium",
		"Rutherfordium",
		"Dubnium",
		"Seaborgium",
		"Bohrium",
		"Hassium",
		"Meitnerium",
	};

	constexpr const char* ElementName(int protonCount)
	{
		return names[protonCount - 1];
	}
	static_assert(ElementName(1) == "Hydrogen");
	
	constexpr ConstStringArray<109> symbols
	{
		"H",
		"He",
		"Li",
		"Be",
		"B",
		"C",
		"N",
		"O",
		"F",
		"Ne",
		"Na",
		"Mg",
		"Al",
		"Si",
		"P",
		"S",
		"Cl",
		"Ar",
		"K",
		"Ca",
		"Sc",
		"Ti",
		"V",
		"Cr",
		"Mn",
		"Fe",
		"Co",
		"Ni",
		"Cu",
		"Zn",
		"Ga",
		"Ge",
		"As",
		"Se",
		"Br",
		"Kr",
		"Rb",
		"Sr",
		"Y",
		"Zr",
		"Nb",
		"Mo",
		"Tc",
		"Ru",
		"Rh",
		"Pd",
		"Ag",
		"Cd",
		"In",
		"Sn",
		"Sb",
		"Te",
		"I",
		"Xe",
		"Cs",
		"Ba",
		"La",
		"Ce",
		"Pr",
		"Nd",
		"Pm",
		"Sm",
		"Eu",
		"Gd",
		"Tb",
		"Dy",
		"Ho",
		"Er",
		"Tm",
		"Yb",
		"Lu",
		"Hf",
		"Ta",
		"W",
		"Re",
		"Os",
		"Ir",
		"Pt",
		"Au",
		"Hg",
		"Tl",
		"Pb",
		"Bi",
		"Po",
		"At",
		"Rn",
		"Fr",
		"Ra",
		"Ac",
		"Th",
		"Pa",
		"U",
		"Np",
		"Pu",
		"Am",
		"Cm",
		"Bk",
		"Cf",
		"Es",
		"Fm",
		"Md",
		"No",
		"Lr",
		"Rf",
		"Db",
		"Sg",
		"Bh",
		"Hs",
		"Mt",
	};

	constexpr const char* ElementSymbol(int protonCount)
	{
		return symbols[protonCount - 1];
	}
	static_assert(ElementSymbol(1)[0] == 'H' && ElementSymbol(1)[1] == '\0');

	namespace Shell
	{
		constexpr ConstIntArray<4> subshells =
		{
			 2, // s
			 6, // p
			10, // d
			14, // f
		};

		constexpr ConstIntArray<7> shellConfigurations =
		{
			0, // K
			1, // L
			2, // M
			3, // N
			3, // O
			2, // P
			0, // Q
		};

		static inline consteval int _ConfigurationElectrons(int configuration)
		{
			int result = 0;
			while (configuration > -1)
				result += subshells[configuration--];
			return result;
		}
		static_assert(_ConfigurationElectrons(1) == 8);

		static inline consteval int _ShellElectrons(int shell)
		{
			int configuration = shellConfigurations[shell];
			int result = _ConfigurationElectrons(configuration);
			return result;
		}
		static_assert(_ShellElectrons(1) == 8);

		constexpr ConstIntArray<7> shells
		{
			_ShellElectrons(0),
			_ShellElectrons(1),
			_ShellElectrons(2),
			_ShellElectrons(3),
			_ShellElectrons(4),
			_ShellElectrons(5),
			_ShellElectrons(6),
		};
		static_assert(shells[4] == 32);
		static_assert(shells[6] == 2);
	}

	struct Atom
	{
		constexpr Atom(int protons, int neutrons, int electrons) :
			protons(protons), neutrons(neutrons), electrons(electrons) {}

		Atom() = default;

		int protons;
		int neutrons;
		int electrons;

		AMU_t Mass()
		{
			return
				protons * g_protonMass +
				neutrons * g_neutronMass +
				electrons * g_electronMass;
		}

		constexpr int ValanceShell() const
		{
			int remaining = electrons;
			int n = 0;
			for (; n < Shell::shells.size(); ++n)
			{
				int shell = Shell::shells[n];
				if (remaining <= shell) break;
				remaining -= shell;
			}
			return n;
		}

		constexpr int ValenceElectrons() const
		{
			int remaining = electrons;
			for (int n = 0; n < Shell::shells.size(); ++n)
			{
				int shell = Shell::shells[n];
				if (remaining <= shell) break;
				remaining -= shell;
			}
			return remaining;
		}

		constexpr bool IsValenceShellFull() const
		{
			int remaining = electrons;
			for (int n = 0; n < Shell::shells.size(); ++n)
			{
				int shell = Shell::shells[n];
				if (remaining == shell) return true;
				if (remaining < shell) return false;
				remaining -= shell;
			}
			return false;
		}

		constexpr bool IsNobleGas() const
		{
			return IsValenceShellFull();
		}

		constexpr bool IsAlkaliMetal() const
		{
			return protons != 1 && ValenceElectrons() == 1;
		}

		constexpr bool IsAlkalineEarthMetal() const
		{
			return ValenceElectrons() == 2;
		}

		// @Todo: add more group testing functions

		constexpr Group GetGroup() const
		{
			if (IsNobleGas()) return Group::NobleGas;
			// Todo
		}

		constexpr const char* Name() const
		{
			return ElementName(protons);
		}
		constexpr const char* Symbol() const
		{
			return ElementSymbol(protons);
		}
	};

	constexpr Atom Hydrogen = { 1,0,1 };
	constexpr Atom Helium   = { 2,2,2 };

	constexpr Atom Carbon   = { 6,6,6 };
	constexpr Atom Nitrogen = { 7,7,7 };
	constexpr Atom Oxygen   = { 8,8,8 };

	// @Todo: Add rest of elements
}

namespace Molecule
{
	template<int SIZE> using AbstractMolecule = ConstArray<Element::Atom, SIZE>;

	template<typename... T>
	consteval auto MakeMolecule(T... _Atom)
	{
		return AbstractMolecule<sizeof...(_Atom)> {_Atom... };
	}
	static_assert(MakeMolecule(Element::Hydrogen, Element::Hydrogen).size() == 2);
}
