#include <camera.hpp>

Camera::Camera(glm::vec3 position, 
  glm::vec3 up, float yaw, float pitch ) : 
  Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY),
  Zoom(ZOOM){
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : 
  Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY),
  Zoom(ZOOM){
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix(){

  return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime){
  float velocity = MovementSpeed * deltaTime;
  if(direction == FORWARD) Position += Up * velocity;// Front * velocity;//
  if(direction == BACKWARD) Position -= Up * velocity;//Front * velocity;//
  if(direction == LEFT) Position -= Right * velocity;
  if(direction == RIGHT) Position += Right * velocity;
}
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constraintPitch){
  xoffset *= MouseSensitivity;
  yoffset *= MouseSensitivity;

  Yaw += xoffset;
  Pitch += yoffset;

  if(constraintPitch){
    if(Pitch > 179.0f) Pitch = 179.0f;
    if(Pitch < -179.0f) Pitch = -179.0f;
  }
  updateCameraVectors();
}
void Camera::ProcessMouseScroll(float yoffset, float deltaTime){
  float velocity = 10*MovementSpeed * deltaTime;
  if(yoffset > 0) Position += Front * velocity;
  if(yoffset < 0) Position -= Front * velocity;
/*   Zoom -= (float)yoffset;
  
  if(Zoom < 1.0f) Zoom = 1.0f;
  if(Zoom > 45.0f) Zoom - 45.0f; */
}
void Camera::updateCameraVectors(){
  glm::vec3 front;
  front.x = -sin(glm::radians(Yaw)) * cos(glm::radians(Yaw));
  front.y = sin(glm::radians(Yaw));
  front.z = sin(glm::radians(Pitch)) * cos(glm::radians(Pitch));
  Front = glm::normalize(front);
  Right = glm::normalize(glm::cross(Front, WorldUp));
  Up = glm::normalize(glm::cross(Right, Front));

  // std::cout<<"Yaw: "<<Yaw;
  // std::cout<<" Pitch: "<<Pitch;
  // std::cout<<" Positionx: "<<Position.x;
  // std::cout<<" Positiony: "<<Position.y;
  // std::cout<<" Positionz: "<<Position.z<<std::endl;
}
Camera::~Camera(){
  
}