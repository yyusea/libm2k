/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <libm2k/m2k.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/analog/m2kanalogout.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/m2kcalibration.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/utils/utils.hpp>
#include <libm2k/utils/device.hpp>
#include <libm2k/logger.hpp>
#include <iio.h>
#include <iostream>

using namespace std;
using namespace libm2k::devices;
using namespace libm2k::analog;
using namespace libm2k::digital;
using namespace libm2k::utils;

M2K::M2K(std::string uri, iio_context* ctx, std::string name) :
	GenericDevice(uri, ctx, name)
{
	std::cout << "I am M2K device " << std::endl;

	initialize();
	setTimeout(UINT_MAX);

	LOG("BOOOMMM");

	m_instancesAnalogIn.clear();
	m_instancesAnalogOut.clear();
	m_instancesPowerSupply.clear();

	scanAllAnalogIn();
	scanAllAnalogOut();
	scanAllPowerSupply();
	scanAllDigital();
	m_calibration = new M2kCalibration(ctx, getAnalogIn(), getAnalogOut());
}

M2K::~M2K()
{
	/* Pre-init call to setup M2k */
	struct iio_device *m2k_fabric = iio_context_find_device(ctx(), "m2k-fabric");
	if (m2k_fabric) {
		auto chn0 = iio_device_find_channel(m2k_fabric, "voltage0", false);
		auto chn1 = iio_device_find_channel(m2k_fabric, "voltage1", false);
		if (chn0 && chn1) {
			iio_channel_attr_write_bool(chn0, "powerdown", true);
			iio_channel_attr_write_bool(chn1, "powerdown", true);
		}

		/* ADF4360 global clock power down */
		iio_device_attr_write_bool(m2k_fabric, "clk_powerdown", true);
	}
	delete m_calibration;
}

void M2K::setTimeout(unsigned int timeout)
{
	iio_context_set_timeout(ctx(), timeout);
}

void M2K::scanAllAnalogIn()
{
	__try {
		Device* aIn = new libm2k::analog::M2kAnalogIn(ctx(), "m2k-adc");
		m_instancesAnalogIn.push_back(aIn);
	} __catch (exception_type &e) {
		std::cout << e.what() << std::endl;
	}
}

void M2K::scanAllAnalogOut()
{
	__try {
		std::vector<std::string> devs = {"m2k-dac-a", "m2k-dac-b"};
		Device* aOut = new libm2k::analog::M2kAnalogOut(ctx(), devs);
		m_instancesAnalogOut.push_back(aOut);
	} __catch (exception_type &e) {
		std::cout << e.what() << std::endl;
	}
}

void M2K::scanAllPowerSupply()
{
	__try {
		Device* pSupply = new libm2k::analog::M2kPowerSupply(ctx(), "ad5627", "ad9963");
		m_instancesPowerSupply.push_back(pSupply);
	} __catch (exception_type &e) {
		std::cout << e.what() << std::endl;
	}
}

void M2K::scanAllDigital()
{
	__try {
		Device* logic = new libm2k::digital::M2kDigital(ctx(), "m2k-logic-analyzer");
		m_instancesDigital.push_back(logic);
	} __catch (exception_type &e) {
		std::cout << e.what() << std::endl;
	}
}

void M2K::calibrate()
{

}

bool M2K::resetCalibration()
{
	__try {
		return m_calibration->resetCalibration();
	} __catch (exception_type &e) {
		throw_exception(EXC_INVALID_PARAMETER, e.what());
	}
}

bool M2K::calibrateADC()
{
	__try {
		return m_calibration->calibrateADC();
	} __catch (exception_type &e) {
		throw_exception(EXC_INVALID_PARAMETER, e.what());
	}
}

bool M2K::calibrateDAC()
{
	__try {
		return m_calibration->calibrateDAC();
	} __catch (exception_type &e) {
		throw_exception(EXC_INVALID_PARAMETER, e.what());
	}
}

double M2K::getAdcCalibrationGain(unsigned int chn)
{
	if (chn >= getAnalogIn()->getNbChannels()) {
		throw_exception(EXC_OUT_OF_RANGE, "No such ADC channel");
	}
	if (chn == 0) {
		return m_calibration->adcGainChannel0();
	} else {
		return m_calibration->adcGainChannel1();
	}
}

int M2K::getAdcCalibrationOffset(unsigned int chn)
{
	if (chn >= getAnalogIn()->getNbChannels()) {
		throw_exception(EXC_OUT_OF_RANGE, "No such ADC channel");
	}
	if (chn == 0) {
		return m_calibration->adcOffsetChannel0();
	} else {
		return m_calibration->adcOffsetChannel1();
	}
}

double M2K::getDacACalibrationGain()
{
	return m_calibration->dacAvlsb();
}

double M2K::getDacBCalibrationGain()
{
	return m_calibration->dacBvlsb();
}

int M2K::getDacACalibrationOffset()
{
	return m_calibration->dacAoffset();
}

int M2K::getDacBCalibrationOffset()
{
	return m_calibration->dacBoffset();
}

M2kAnalogIn* M2K::getAnalogIn()
{
	auto aIn = dynamic_cast<libm2k::analog::M2kAnalogIn*>(
				m_instancesAnalogIn.at(0));
	if (aIn) {
		return aIn;
	} else {
		return nullptr;
	}
}

M2kAnalogIn* M2K::getAnalogIn(string dev_name)
{
	for (Device* d : m_instancesAnalogIn) {
		if (d->getName() == dev_name) {
			libm2k::analog::M2kAnalogIn* analogIn =
				dynamic_cast<libm2k::analog::M2kAnalogIn*>(d);
			if (analogIn) {
				return analogIn;
			}
		}
	}
	return nullptr;
}

M2kPowerSupply* M2K::getPowerSupply()
{
	M2kPowerSupply* pSupply = dynamic_cast<M2kPowerSupply*>(m_instancesPowerSupply.at(0));
	if (pSupply) {
		return pSupply;
	}
	throw_exception(EXC_INVALID_PARAMETER, "No M2K power supply");
}

M2kDigital* M2K::getDigital()
{
	M2kDigital* logic = dynamic_cast<M2kDigital*>(m_instancesDigital.at(0));
	if (logic) {
		return logic;
	}
	throw_exception(EXC_INVALID_PARAMETER, "No M2K digital device found");
}

M2kAnalogOut* M2K::getAnalogOut()
{
	for (Device* d : m_instancesAnalogOut) {
		libm2k::analog::M2kAnalogOut* analogOut =
				dynamic_cast<M2kAnalogOut*>(d);
		if (analogOut) {
			return analogOut;
		}
	}
	return nullptr;
}

std::vector<M2kAnalogIn*> M2K::getAllAnalogIn()
{
	std::vector<libm2k::analog::M2kAnalogIn*> allAnalogIn = {};
	for (Device* inst : m_instancesAnalogIn) {
		libm2k::analog::M2kAnalogIn* analogIn =
				dynamic_cast<M2kAnalogIn*>(inst);
		if (analogIn) {
			allAnalogIn.push_back(analogIn);
		}
	}
	return allAnalogIn;
}

std::vector<M2kAnalogOut*> M2K::getAllAnalogOut()
{
	std::vector<libm2k::analog::M2kAnalogOut*> allAnalogOut;
	for (Device* inst : m_instancesAnalogOut) {
		libm2k::analog::M2kAnalogOut* analogOut =
				dynamic_cast<libm2k::analog::M2kAnalogOut*>(inst);
		if (analogOut) {
			allAnalogOut.push_back(analogOut);
		}
	}
	return allAnalogOut;
}

void M2K::initialize()
{
	std::string hw_rev = Utils::getHardwareRevision(ctx());

	std::shared_ptr<Device> m_ad9963 = make_shared<Device>(ctx(), "ad9963");
	std::shared_ptr<Device> m_m2k_fabric = make_shared<Device>(ctx(), "m2k-fabric");
	int config1 = 0x05;
	int config2 = 0x05;

	if (hw_rev == "A") {
		config1 = 0x1B; // IGAIN1 +-6db  0.25db steps
		config2 = 0x1B;
	}

	/* Configure TX path */
	m_ad9963->writeRegister(0x68, config1);
	m_ad9963->writeRegister(0x6B, config2);
	m_ad9963->writeRegister(0x69, 0x1C);  // IGAIN2 +-2.5%
	m_ad9963->writeRegister(0x6C, 0x1C);
	m_ad9963->writeRegister(0x6A, 0x20);  // IRSET +-20%
	m_ad9963->writeRegister(0x6D, 0x20);

	/* Pre-init call to setup M2k */
	m_m2k_fabric->setBoolValue(0, false, "powerdown", false);
	m_m2k_fabric->setBoolValue(1, false, "powerdown", false);
	m_m2k_fabric->setBoolValue(false, "clk_powerdown");
}
