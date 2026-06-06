#pragma once

namespace pybricksDriver::messaging
{
	template<typename T>
	requires std::is_signed_v<T>
	Unmodulator<T>::Unmodulator(const T& maxVal)
		:_maxVal(maxVal)
		,_lastRawVal{}
	,_hasLastRawVal(false)
	,_wrapAroundCount(0)
	{}

	template<typename T>
	requires std::is_signed_v<T>
	Unmodulator<T>::~Unmodulator() = default;


	template<typename T>
	requires std::is_signed_v<T>
	T Unmodulator<T>::operator()(const T& rawVal) {
		if (!_hasLastRawVal) {
			_lastRawVal = rawVal;
			_hasLastRawVal = true;
			return rawVal;
		}

		T delta = rawVal - _lastRawVal;

		_lastRawVal = rawVal;

		if (delta > _maxVal / 2) {
			--_wrapAroundCount;
		}else if (delta < -_maxVal / 2) {
			++_wrapAroundCount;
		}

		return rawVal + _wrapAroundCount * _maxVal;
	}
}