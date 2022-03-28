/*
 * display.h
 *
 *  Created on: 13.05.2017
 *      Author: jane
 */

#ifndef ABSTRACTDISPLAY_H_
#define ABSTRACTDISPLAY_H_

#include <Arduino.h>

namespace forms{

class AbstractDisplay {
public:
//	virtual ~Display();
	virtual void setCursorXY(__signed char x, __signed char y);
	virtual void setInvertedMode(bool inverted);
//	virtual void setFontColor(unsigned int color);
//	virtual void setBackgrowndColor(unsigned int color);
	virtual void print(String msg);
	virtual void println(String msg);
	virtual void setDrawCharMask(unsigned char mask); // спецэффекты на текст
	virtual void sendRawScreeData(unsigned char *data);// отправка данных на экран. 1к данных

    virtual int getWidth();
    virtual int getHeight();
	virtual int getBitsPerPixel();

	/**
	 * Подготовить дисплей. отрисовать рамку или что там еще
	 */
	virtual void prepare();

	/**
	 * Очистить экран, не стирая рамки или чего там еще
	 */
	virtual void clearDisplay();

};

}
#endif /* ABSTRACTDISPLAY_H_ */
