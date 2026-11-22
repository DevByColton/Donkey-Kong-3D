#ifndef CAMERA_H
#define CAMERA_H

struct Camera
{
    float pitch = {};
    float yaw = {};
    
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 front = {};
    DirectX::XMFLOAT3 up = {};
};

internal_link void
ApplyScaleRotationTranslation(Transform* transform, ModelViewProjectionData* modelViewProjectionData)
{
    DirectX::XMMATRIX model = DirectX::XMMatrixIdentity();

    //
    // NOTE: Scale
    //
    DirectX::XMMATRIX scale = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(
        transform->scale.x, 
        transform->scale.y, 
        transform->scale.z, 
        1.f)
    );
    model = DirectX::XMMatrixMultiply(model, scale);

    //
    // NOTE: Rotate
    //
    // From the microsoft docs, negating since I am using a RH coordinate system
    // Angles are measured clockwise when looking along the rotation axis toward the origin. 
    // This is a left-handed coordinate system. 
    // To use right-handed coordinates, negate all three angles
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
        -DirectX::XMConvertToRadians(transform->rotation.x),
        -DirectX::XMConvertToRadians(transform->rotation.y),
        -DirectX::XMConvertToRadians(transform->rotation.z)
    );
    model = DirectX::XMMatrixMultiply(model, rotation);

    //
    // NOTE: Translate
    //
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(
        transform->position.x, 
        transform->position.y, 
        transform->position.z
    );
    model = DirectX::XMMatrixMultiply(model, translation);

    DirectX::XMStoreFloat4x4(&modelViewProjectionData->model, DirectX::XMMatrixTranspose(model));
}

internal_link void
ApplyView(ModelViewProjectionData* modelViewProjectionData, Camera* camera = 0)
{
    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    
    if (camera)
    {
        DirectX::FXMVECTOR position = DirectX::XMLoadFloat3(&camera->position);
        DirectX::FXMVECTOR front = DirectX::XMLoadFloat3(&camera->front);
        DirectX::FXMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
        DirectX::FXMVECTOR focalPoint = DirectX::XMVectorAdd(position, front);
        view = DirectX::XMMatrixLookAtRH(position, focalPoint, up);
    }

    DirectX::XMStoreFloat4x4(&modelViewProjectionData->view, DirectX::XMMatrixTranspose(view)); 
}

internal_link void
OrthographicProjection(ModelViewProjectionData* modelViewProjectionData, unsigned int viewWidth, unsigned int viewHeight)
{
    float nearPlane = 0.01f;
    float farPlane = 100.f;

    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicRH((float)viewWidth, (float)viewHeight, nearPlane, farPlane);
    DirectX::XMStoreFloat4x4(&modelViewProjectionData->projection, DirectX::XMMatrixTranspose(projection));
}

internal_link void
PerspectiveProjection(ModelViewProjectionData* modelViewProjectionData, unsigned int viewWidth, unsigned int viewHeight)
{
    float fov = DirectX::XMConvertToRadians(65.f);
    float aspectRatio = (float)viewWidth / (float)viewHeight;
    float nearPlane = 0.01f;
    float farPlane = 100.f;

    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
    DirectX::XMStoreFloat4x4(&modelViewProjectionData->projection, DirectX::XMMatrixTranspose(projection));
}

internal_link void
HandleCameraMovement(GameState* gameState, GameInput* previousInput, GameInput* newInput, Camera* camera, 
                     float cameraSpeed, float mouseSensitivity, float thumbstickSensitivity)
{
    cameraSpeed *= gameState->deltaTime;
    mouseSensitivity *= gameState->deltaTime;
    thumbstickSensitivity *= gameState->deltaTime;

    //
    // NOTE: Handle look
    //
    float yaw = camera->yaw;
    float pitch = camera->pitch;

    if (gameState->isFocused)
    {
        // NOTE: Mouse look
        float xMouseOffset = (float)newInput->mouse.x - (float)previousInput->mouse.x;
        float yMouseOffset = (float)previousInput->mouse.y - (float)newInput->mouse.y;

        // NOTE: Controller look
        float xRightThumbstick = 0.f;
        float yRightThumbstick = 0.f;
        for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
        {
            GameControllerInput* newController = newInput->controllers + controllerIndex;
            
            if (newController->isConnected)
            {
                xRightThumbstick += newController->rightThumbstick.x;
                yRightThumbstick += newController->rightThumbstick.y;
            }
        }

        yaw += ((xMouseOffset * mouseSensitivity) + (xRightThumbstick * thumbstickSensitivity));
        pitch += ((yMouseOffset * mouseSensitivity) + (yRightThumbstick * thumbstickSensitivity));

        // NOTE: Allow the camera to look up and down but clamping before hitting 90 where the LookAt would flip
        if(pitch > 89.f)
        {
            pitch = 89.f;
        }
        if(pitch < -89.f)
        {
            pitch = -89.f;
        }
    }

    // NOTE: Store yaw and pitch before converting to radians
    camera->yaw = yaw;
    camera->pitch = pitch;

    yaw = DirectX::XMConvertToRadians(yaw);
    pitch = DirectX::XMConvertToRadians(pitch);
    float x = DirectX::XMScalarCos(yaw) * DirectX::XMScalarCos(pitch);
    float y = DirectX::XMScalarSin(pitch);
    float z = DirectX::XMScalarSin(yaw) * DirectX::XMScalarCos(pitch);
    DirectX::XMVECTOR direction = DirectX::XMVectorSet(x, y, z, 0.f);

    //
    // NOTE: Handle directional movement
    //
    float moveRight = 0.0f;
    float moveFront = 0.0f;
    for (unsigned int controllerIndex = 0; controllerIndex < ARRAYSIZE(newInput->controllers); ++controllerIndex)
    {
        GameControllerInput* controller = newInput->controllers + controllerIndex;
        if (controller->isConnected)
        {
            moveRight += controller->leftThumbstick.x;
            moveFront += controller->leftThumbstick.y;
        }
    }

    DirectX::FXMVECTOR previousPosition = DirectX::XMLoadFloat3(&camera->position);
    DirectX::FXMVECTOR front = DirectX::XMVector3Normalize(direction);
    DirectX::FXMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
    DirectX::FXMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(front, up));
    
    // NOTE: Factor in summed controller movement
    DirectX::XMVECTOR newMovement = DirectX::XMVectorAdd(DirectX::XMVectorScale(right, moveRight), DirectX::XMVectorScale(front, moveFront));

    // NOTE: Keyboard movement
    if (newInput->keyboard.w.endedDown)
    {
        newMovement = DirectX::XMVectorAdd(newMovement, front);
    }
    if (newInput->keyboard.a.endedDown)
    {
        newMovement = DirectX::XMVectorSubtract(newMovement, right);
    }
    if (newInput->keyboard.s.endedDown)
    {
        newMovement = DirectX::XMVectorSubtract(newMovement, front);
    }
    if (newInput->keyboard.d.endedDown)
    {
        newMovement = DirectX::XMVectorAdd(newMovement, right);
    }
    
    float newMovementLengthSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(newMovement));
    if (newMovementLengthSq > 1.f)
    {
        newMovement = DirectX::XMVector3Normalize(newMovement);
    }

    DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(previousPosition, DirectX::XMVectorScale(newMovement, cameraSpeed));
    DirectX::XMStoreFloat3(&camera->position, newPosition);
    DirectX::XMStoreFloat3(&camera->front, front);
}

#endif // CAMERA_H