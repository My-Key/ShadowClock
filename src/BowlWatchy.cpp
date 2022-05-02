#include "BowlWatchy.h"

const float VOLTAGE_MIN = 3.4;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

const float MINUTE_ANGLE = 360 / 60;
const float HOUR_ANGLE = 360 / 12;

const float CONE_RADIUS = 10;
const float CONE_HEIGHT = 80;

const Vector3<float> CONE_CENTER = {99.5, 99.5, 0};
const Vector3<float> NORMAL_DIR = {0, 0, 1};
const Vector3<float> CONE_TOP = CONE_CENTER + NORMAL_DIR * CONE_HEIGHT;
const Vector3<float> VIEW_DIR = {0,0,1};

void BowlWatchy::drawWatchFace()
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  drawTime();
}

int BowlWatchy::getBatteryFill(int steps)
{
  float VBAT = getBatteryVoltage();

  // 12 battery states
  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * steps);
  if (batState > steps)
    batState = steps;
  if (batState < 0)
    batState = 0;

  return batState;
}

static float clamp(const float& value)
{
  float outValue = value;
  if (outValue > 1.0)
        outValue = 1.0;
  else if (outValue < 0.0)
        outValue = 0.0;

  return outValue;
}

static float sign (const Vector3<float>& p1, const Vector3<float>& p2, const Vector3<float>& p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static bool PointInTriangle (const Vector3<float>& pt, const Vector3<float>& v1, const Vector3<float>& v2, const Vector3<float>& v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

static void BlinPhong(const int x, const int y, const Vector3<float>& normal, const Vector3<float>& lightDir, const Vector3<float>& halfView, const float& gloss, 
const Vector3<float>& coneBase1, const Vector3<float>& coneBase2, const Vector3<float>& coneBase3, float& intensity, float& specularIntensity)
{
  float shadow = 1.0;

  Vector3<float> point = {x,y,0};

  if ((point - CONE_CENTER).sqrMagnitude() > CONE_RADIUS * CONE_RADIUS && PointInTriangle(point, coneBase1, coneBase2, coneBase3))
    shadow = 0;

  float NdotL = Vector3<float>::dotProduct(normal, lightDir);
  intensity += clamp(NdotL) * shadow;

  float NdotH = Vector3<float>::dotProduct(normal, halfView);
  specularIntensity += pow(clamp(NdotH), gloss / 8) * shadow;
}

void BowlWatchy::drawTime()
{
  int hour = currentTime.Hour;
  int minute = currentTime.Minute;

  Vector3<float> minuteLight = {0,-1,1};
  minuteLight = Vector3<float>::rotateVector(minuteLight, MINUTE_ANGLE * minute);
  minuteLight.normalize();

  Vector3<float> coneBaseMinute1 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 1);
  coneBaseMinute1.z = 0;
  coneBaseMinute1.normalize();
  coneBaseMinute1.scale(CONE_RADIUS);
  coneBaseMinute1 = coneBaseMinute1 + CONE_CENTER;

  Vector3<float> coneBaseMinute2 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 3);
  coneBaseMinute2.z = 0;
  coneBaseMinute2.normalize();
  coneBaseMinute2.scale(CONE_RADIUS);
  coneBaseMinute2 = coneBaseMinute2 + CONE_CENTER;

  Vector3<float> coneBaseMinute = CONE_TOP + minuteLight * (CONE_HEIGHT / Vector3<float>::dotProduct(NORMAL_DIR, minuteLight));

  Vector3<float> minuteHalfView = minuteLight + VIEW_DIR;
  minuteHalfView.normalize();

  Vector3<float> hourLight = {0,-0.5,1};
  hourLight = Vector3<float>::rotateVector(hourLight, HOUR_ANGLE * ((hour % 12) + minute/60.0) );
  hourLight.normalize();

  Vector3<float> coneBaseHour1 = Vector3<float>::rotateVectorByRightAngle(hourLight, 1);
  coneBaseHour1.z = 0;
  coneBaseHour1.normalize();
  coneBaseHour1.scale(CONE_RADIUS);
  coneBaseHour1 = coneBaseHour1 + CONE_CENTER;

  Vector3<float> coneBaseHour2 = Vector3<float>::rotateVectorByRightAngle(hourLight, 3);
  coneBaseHour2.z = 0;
  coneBaseHour2.normalize();
  coneBaseHour2.scale(CONE_RADIUS);
  coneBaseHour2 = coneBaseHour2 + CONE_CENTER;

  Vector3<float> coneBaseHour = CONE_TOP + hourLight * (CONE_HEIGHT / Vector3<float>::dotProduct(NORMAL_DIR, hourLight));
  
  Vector3<float> hourHalfView = hourLight + VIEW_DIR;
  hourHalfView.normalize();
  
  display.startWrite();

  for (int y = 0; y < 200; y++)
  {
    for (int x = 0; x < 200; x++)
    {
      int index = x * 3 + 200 * 3 * y;
      Vector3<float> normal = {Normal[index], Normal[index + 1], Normal[index + 2]};
      normal = normal * (2 / 255.0);
      normal = normal - 1;
      normal.x *= -1;
      normal.normalize();

      float intensity = 0;
      float specularIntensity = 0;
      float gloss = Gloss[x + 200 * y];

      BlinPhong(x, y, normal, minuteLight, minuteHalfView, gloss, coneBaseMinute, coneBaseMinute1, coneBaseMinute2, intensity, specularIntensity);
      BlinPhong(x, y, normal, hourLight, hourHalfView, gloss, coneBaseHour, coneBaseHour1, coneBaseHour2, intensity, specularIntensity);

      bool white = (intensity * Diffuse[x + 200 * y] + specularIntensity * gloss) * 0.5 > BlueNoise200[y * 200 + x];
      display.drawPixel(x, y, white ? GxEPD_WHITE : GxEPD_BLACK);
    }
  }
  
  display.endWrite();
}
