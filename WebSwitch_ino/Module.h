#ifndef Module_h
#define Module_h

#include "Arduino.h"

enum Type {SWITCH = 0, LIGHT = 1, DIMMER = 2, ONOFFM = 3};

class Module
{
        protected:
                Type m_type;
                int m_state;

        public:
                Module();
                /*Module(Type p_Type, int p_State)
                {
                    m_type = p_Type;
                    m_state = p_State;
                }*/
                //virtual ~Module(void);

                Type GetType()
                {
                        return m_type;
                }


};
#endif
