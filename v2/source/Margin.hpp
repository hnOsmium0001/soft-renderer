#pragma once

template <class T>
class Margin {
public:
	union {
		struct {
			T left;
			T top;
			T right;
			T bottom;
		};
		T values[4];
	};

public:
	Margin()
		: left{ 0 }, top{ 0 }, right{ 0 }, bottom{ 0 } {
	}

	Margin(T value)
		: left{ value }, top{ value }, right{ value }, bottom{ value } {
	}

	Margin(T left, T top, T right, T bottom)
		: left{ left }
		, top{ top }
		, right{ right }
		, bottom{ bottom } {
	}

	template <class TTarget>
	Margin<TTarget> Cast() const {
		return {
			static_cast<TTarget>(left),
			static_cast<TTarget>(top),
			static_cast<TTarget>(right),
			static_cast<TTarget>(bottom),
		};
	}
};
