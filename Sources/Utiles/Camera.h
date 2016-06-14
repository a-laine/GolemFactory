#pragma once

#include <atomic>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
  
#include "../Events/EventHandler.h"
#include "Mutex.h"


class Camera
{
    public:
        //  Miscellaneous
        enum CameraMode
        {
            FREEFLY = 0,
            TRACKBALL = 1,
            CINEMATIC = 2,
            ISOMETRIC = 3,
            MODE_MASK = 0x03
        };
        enum CinematicState
        {
            PLAY = 1<<2
        };
        //

        //  Default
        Camera();
        ~Camera();
        //

        //  Public functions
        void animate(float elapseTime,bool goForw = false,bool goBack = false,
                     bool goLeft = false,   bool goRight = false,
                     bool option1 = false,      bool option2 = false);
		glm::mat4 getModelViewMatrix();
        void move(glm::dvec3 v);

        void pause();
        void play();
        //

        //  Set/get functions
        void setMode(CameraMode mode);
        void setSpeed(float s);
        void setRadiusMin(double r);
        void setRadiusMax(double r);
        void setRadius(double r);
        void setAllRadius(double r,double rmin,double rmax);
        void setPosition(glm::dvec3 pos);
        void setOrientation(glm::dvec3 orientation);
        void setTarget(glm::dvec3 target);
        void setFrustrumAngleVertical(float angle);
        void setFrustrumAngleHorizontal(float angle);
        void setSensitivity(float sens);

        CameraMode getMode();
		glm::dvec3 getTarget();
		glm::dvec3 getLeft();
		glm::dvec3 getForward();
		glm::dvec3 getVertical();
		glm::dvec3 getPosition();
        float getSpeed();
        double getRadiusMin();
        double getRadiusMax();
        double getRadius();
        float getFrustrumAngleVertical();
        float getFrustrumAngleHorizontal();
        float getSensitivity();
        //

    private:
        //  Private functions
        void vectorsFromAngles(glm::dvec3 target = glm::dvec3(0,0,0));
        void anglesFromVectors();
        void boundingRadius();
        //

        //  Attributes
        uint8_t configuration;
        Mutex mutex;

		glm::dvec3 position,
                   forward,
                   left,
                   vertical;

        double radius,
               radiusMin,
               radiusMax;

        float sensivity,
              speedMag,
              frustrumAngleHorizontal,
              frustrumAngleVertical,
              phi,
              teta;
        //
};
