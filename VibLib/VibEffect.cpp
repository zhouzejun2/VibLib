//
//  VibEffect.cpp
//  VibLib
//
//  Created by Lucas Duroj on 23/07/15.
//  Copyright (c) 2015 Lucas Duroj. All rights reserved.
//

#include "VibEffect.h"
#include "VibDevice.h"
#include <stdlib.h>
#include <CoreFoundation/CFUUID.h>

CFUUIDRef effectTypeToNative(VibEffectType vibType) {
	switch (vibType) {
		case EFFECT_TYPE_SINE:
			return kFFEffectType_Sine_ID;
			break;
		case EFFECT_TYPE_SQUARE:
			return kFFEffectType_Square_ID;
			break;
		case EFFECT_TYPE_TRIANGLE:
			return kFFEffectType_Triangle_ID;
			break;
		case EFFECT_TYPE_SAWTOOTH_DOWN:
			return kFFEffectType_SawtoothDown_ID;
			break;
		case EFFECT_TYPE_SAWTOOTH_UP:
			return kFFEffectType_SawtoothUp_ID;
			break;
	}
	return kFFEffectType_Sine_ID;
}

VibEffect::VibEffect(const VibDevice* device, string name, const VibEffectData &data) :
data(data),
device(device),
name(name),
ffEffectRef(NULL)
{
	createFFEffect();
	updateFFEffect();
	
	//Create FF effect
	HRESULT ret = FFDeviceCreateEffect(device->ffDevice, effectTypeToNative(data.type), &ffEffect, &ffEffectRef);
	if(ret != FF_OK) {
		printf("VibEffect: Failed to create effect");
	}
}

VibEffect::~VibEffect() {
	freeFFEffect();
}

void VibEffect::start() {
	HRESULT ret = FFEffectStart(ffEffectRef, 1, 0);
	if (ret != FF_OK) {
		printf("VibEffect: Failed to start effter");
	}
}

void VibEffect::updateCord() {
	
	// if no direction as assigne default to SPHERICAL
	if (data.effect.axesCount == 0) {
		ffEffect.dwFlags |= FFEFF_SPHERICAL;
		ffEffect.rglDirection = NULL;
	}
	
	LONG* rglDir = ffEffect.rglDirection;
	if(!rglDir) {
		rglDir = (LONG*)malloc(sizeof(LONG) * data.effect.axesCount);
	}
	memset(rglDir, 0, sizeof(LONG) * data.effect.axesCount);
	ffEffect.rglDirection = rglDir;
	
	switch (data.effect.axisCord) {
		case FFEFF_POLAR:
			ffEffect.dwFlags |= FFEFF_POLAR;
			rglDir[0] = data.effect.axes[0];
			break;
		case FFEFF_CARTESIAN:
			ffEffect.dwFlags |= FFEFF_CARTESIAN;
			rglDir[0] = data.effect.axes[0];
			if (data.effect.axesCount > 1)
				rglDir[1] = data.effect.axes[1];
			if (data.effect.axesCount > 2)
				rglDir[2] = data.effect.axes[2];
			break;
		case FFEFF_SPHERICAL:
			ffEffect.dwFlags |= FFEFF_SPHERICAL;
			rglDir[0] = data.effect.axes[0];
			if (data.effect.axesCount > 1)
				rglDir[1] = data.effect.axes[1];
			if (data.effect.axesCount > 2)
				rglDir[2] = data.effect.axes[2];
			break;
		default:
			printf("VibEffect: Unknown direction type.");
	}
}

#define CCONVERT(x)   (((x) > 0x7FFF) ? 10000 : ((x)*10000) / 0x7FFF)
#define CONVERT(x)    (((x)*10000) / 0x7FFF)

void VibEffect::updateEffect(const VibEffectData& data) {
	this->data = data;
	updateFFEffect();
	
	static const FFEffectParameterFlag flags = FFEP_DIRECTION | FFEP_DURATION | FFEP_ENVELOPE | FFEP_STARTDELAY | FFEP_TYPESPECIFICPARAMS;
	HRESULT ret = FFEffectSetParameters(ffEffectRef, &ffEffect, flags);
	if (ret != FF_OK) {
		printf("VibEffect: Unable to update effect.");
	}
}

void VibEffect::createFFEffect() {
	memset(&ffEffect, 0, sizeof(FFEFFECT));
	ffEffect.rgdwAxes = NULL;
	ffEffect.lpvTypeSpecificParams = NULL;
	ffEffect.lpvTypeSpecificParams = NULL;
}

void VibEffect::updateFFEffect() {
	ffEffect.dwSize = sizeof(FFEFFECT);
	ffEffect.dwSamplePeriod = data.effect.samplePeriod;
	ffEffect.dwGain = data.effect.gain;
	ffEffect.dwFlags = FFEFF_OBJECTOFFSETS;
	
	ffEffect.dwDuration = data.effect.duration * 1000;
	ffEffect.dwStartDelay = data.effect.delay * 1000;    /* In microseconds. */
	
	ffEffect.cAxes = device->axisCount;
	if (ffEffect.cAxes > 0) {
		DWORD* axes = ffEffect.rgdwAxes;
		if(!axes)
			axes = (DWORD*)malloc(sizeof(DWORD) * ffEffect.cAxes);
		axes[0] = device->axes[0];
		if (ffEffect.cAxes > 1) {
			axes[1] = device->axes[1];
		}
		if (ffEffect.cAxes > 2) {
			axes[2] = device->axes[2];
		}
		ffEffect.rgdwAxes = axes;
	}
	
	
	//update periodic data
	if(data.type == EFFECT_TYPE_SINE
	   || data.type == EFFECT_TYPE_SQUARE
	   || data.type == EFFECT_TYPE_TRIANGLE
	   || data.type == EFFECT_TYPE_SAWTOOTH_UP
	   || data.type == EFFECT_TYPE_SAWTOOTH_DOWN) {
		FFPERIODIC* periodic = (FFPERIODIC*)ffEffect.lpvTypeSpecificParams;
		if(!periodic) {
			periodic = (FFPERIODIC*)malloc(sizeof(FFPERIODIC));
		}
		memset(periodic, 0, sizeof(FFPERIODIC));
		periodic->dwMagnitude = CONVERT(data.periodic.magnitude);
		periodic->lOffset = CONVERT(data.periodic.offset);
		periodic->dwPhase = data.periodic.phase;
		periodic->dwPeriod = data.periodic.period * 1000;
		
		ffEffect.cbTypeSpecificParams = sizeof(FFPERIODIC);
		ffEffect.lpvTypeSpecificParams = (void*)periodic;
		
		updateCord();
	}
	
}

void VibEffect::freeFFEffect() {
	
	if(ffEffect.rgdwAxes)
		free(ffEffect.rgdwAxes);
	if(ffEffect.lpvTypeSpecificParams)
		free(ffEffect.lpvTypeSpecificParams);
	if(ffEffect.rglDirection)
		free(ffEffect.rglDirection);
	
	if(ffEffectRef != NULL) {
		FFEffectUnload(ffEffectRef);
		ffEffectRef = NULL;
	}
}