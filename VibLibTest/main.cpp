//
//  main.cpp
//  VibLibTest
//
//  Created by Lucas Duroj on 23/07/15.
//  Copyright (c) 2015 Lucas Duroj. All rights reserved.
//

#include <stdio.h>
#include "vibLib/VibLib.h"

int main(void) {
	
	VibLib* vibLib = new VibLib();
	vibLib->scanDevices();
	
	printf("Scaning for deveices\n");
	int deviceCount = vibLib->getDeviceCount();
	if(deviceCount > 0) {
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			printf("Found device: %s\n", device->name);
		}
		
		printf("Test 1: Create effect with envelope\n");
		VibEffectData data = VibEffectData();
		data.periodic.period = 1000;
		data.periodic.magnitude = 10000;
		data.effect.duration = 2000;
		
		data.envelope.attackTime = 500;
		data.envelope.attackLevel = 0;
		
		data.envelope.fadeTime = 1500;
		data.envelope.fadeLevel = 0;
		
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			auto effect = device->createEffect("vibrate", data);
			effect->start();
			printf("Test 1.%i: Play device num: %i \n", i+1, i);
			sleep(2);
		}
		
		sleep(1);
		
		printf("Test 2: Update effect \n");
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			
			data.effect.duration = 2000;
			data.periodic.magnitude = 20000;
			
			auto effect = device->getEffect("vibrate");
			if(effect != NULL)
				effect->updateEffect(data);
			
			effect->start();
		}
		sleep(3);
		
		printf("Test 4: Create costant effect with axis\n");
		data = VibEffectData();
		data.type = EFFECT_TYPE_CONSTANT;
		data.constant.magnitude = 30000;
		data.effect.duration = 1000;
		
		data.effect.axisCord = FFEFF_POLAR;
		data.effect.axes[0] = 0;
		data.effect.axes[1] = -10000;
		data.effect.axes[2] = 0;
		
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			auto effect = device->createEffect("vibrate-constant", data);
			effect->start();
		}
		sleep(2);
		
		printf("Test 5: Update effect type \n");
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			
			auto effect = device->getEffect("vibrate-constant");
			data = effect->data;
			data.type = EFFECT_TYPE_SAWTOOTH_UP;
			data.effect.duration = 500;
			data.periodic.magnitude = 10000;
			
			if(effect != NULL)
				effect->updateEffect(data);
			effect->start();
		}
		sleep(1);
		
		
		printf("Test 6: Deletes effects \n");
		for (int i = 0; i < deviceCount; ++i) {
			auto device = vibLib->getDevice(i);
			device->deleteEffect("vibrate");
			device->deleteEffect("vibrate-constant");
		}
	} else {
		printf("Did not find any devices\n");
	}
	
	printf("Test done");
	
	delete vibLib;
}