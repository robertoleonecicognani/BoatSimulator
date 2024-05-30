#include "boat_runner.hpp"
#include "collision_box.hpp"

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

const std::string WINDOW_TITLE = "Boat Runner";

const std::string VERT_SHADER_PATH = "shaders/vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/frag.spv";

const std::string SKYBOX_VERT_SHADER_PATH = "shaders/skyboxVert.spv";
const std::string SKYBOX_FRAG_SHADER_PATH = "shaders/skyboxFrag.spv";

const std::string TEXT_VERT_SHADER_PATH = "shaders/textVert.spv";
const std::string TEXT_FRAG_SHADER_PATH = "shaders/textFrag.spv";

const std::string BOAT_MODEL_PATH = "models/boat.obj";
const std::string BOAT_TEXTURE_PATH = "textures/boat.bmp";

const std::string ROCK1_MODEL_PATH = "models/rock1.obj";
const std::string ROCK1_TEXTURE_PATH = "textures/rock1.png";

const std::string ROCK2_MODEL_PATH = "models/rock2.obj";
const std::string ROCK2_TEXTURE_PATH = "textures/rock2.jpg";

const std::string OCEAN_MODEL_PATH = "models/ocean.obj";
const std::string OCEAN_TEXTURE_PATH = "textures/ocean.png";

const std::string WIN_MODEL_PATH = "models/textWin.obj";
const std::string WIN_TEXTURE_PATH = "textures/text.png";

const std::string LOSE_MODEL_PATH = "models/textLose.obj";
const std::string LOSE_TEXTURE_PATH = "textures/text.png";

const std::string RESTART_MODEL_PATH = "models/textRestart.obj";
const std::string RESTART_TEXTURE_PATH = "textures/text.png";

const std::string SKYBOX_MODEL_PATH = "models/skyboxCube.obj";
const std::string SKYBOX_TEXTURES_PATH[] = {"textures/sky/bkg1_right.png",
                                            "textures/sky/bkg1_left.png",
                                            "textures/sky/bkg1_top.png",
                                            "textures/sky/bkg1_bot.png",
                                            "textures/sky/bkg1_front.png",
                                            "textures/sky/bkg1_back.png"};

const int ROCK1_NUMBER = 6;
const int ROCK2_NUMBER = 6;

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 40.0f;

const float MIN_X = -35.0f;
const float SPAWN_LIMIT_X = -20.0f;
const float MAX_X = 3.25f;

const float MIN_Z = -10.0f;
const float MAX_Z = 10.0f;

const float HORIZONTAL_SPEED = 1.8f;
const float VERTICAL_SPEED = 5.0f;
const float VERTICAL_SPEED_INCREMENT = 0.05f;

const float OCEAN_SPEED = 0.6f;
const float OCEAN_SPEED_INCREMENT = 0.0025f;
const glm::vec3 OCEAN_INIT_POS = glm::vec3(-30.0f, -0.13f, -24.0f);

const int MAX_POSITION_GENERATION = 10;
const float MIN_ROCK_DISTANCE = 0.7f;

const int WIN_POINTS = 200;

const glm::vec3 WIN_TEXT_POSITION = glm::vec3(-0.475, -0.5, 0);
const glm::vec3 LOSE_TEXT_POSITION = glm::vec3(-0.45, -0.5, 0);
const glm::vec3 RESTART_TEXT_POSITION = glm::vec3(-0.35, -0.2, 0);
const glm::vec3 OUT_TEXT_POSITION = glm::vec3(-2, -2, 0);

struct Game
{
    bool started = false;
    int points = 0;
    int highscore = 0;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct SkyBoxUniformBufferObject
{
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

class BoatRunner : public BaseProject
{
protected:
    Game game;

    DescriptorSetLayout descSetLayout;
    Pipeline pipeline;

    DescriptorSetLayout textDescSetLayout;
    Pipeline textPipeline;

    DescriptorSetLayout skyboxDescSetLayout;
    Pipeline skyboxPipeline;
    SkyBoxModel skybox = {SKYBOX_MODEL_PATH, std::vector<std::string>(std::begin(SKYBOX_TEXTURES_PATH), std::end(SKYBOX_TEXTURES_PATH))};

    std::vector<Object> objects = {};
    std::vector<Text> texts = {};

    void setWindowParameters()
    {
        windowWidth = WINDOW_WIDTH;
        windowHeight = WINDOW_HEIGHT;
        windowTitle = WINDOW_TITLE;
        initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
    }

    // Setups objects instances
    void setupObjects()
    {
        glm::vec3 generatedPosition;
        float generatedScale;

        // Boat
        Object boat = {BOAT_MODEL_PATH, BOAT_TEXTURE_PATH, 0.0012f};
        objects.push_back(boat);

        ObjectInstance boatInstance = {Boat, glm::vec3(2.5f, -0.1f, 0.0f), glm::vec3(0), glm::vec3(boat.defaultScale)};
        objects.back().instances.push_back(boatInstance);

        // Rock1
        Object rock1 = {ROCK1_MODEL_PATH, ROCK1_TEXTURE_PATH, 0.15f};
        objects.push_back(rock1);

        for (int i = 0; i < ROCK1_NUMBER; ++i)
        {
            std::tie(generatedPosition, generatedScale) = generateRandomRockSpawn(rock1);
            ObjectInstance rockInstance = {Rock, generatedPosition, glm::vec3(0), glm::vec3(generatedScale)};
            objects.back().instances.push_back(rockInstance);
        }

        // Rock2
        Object rock2 = {ROCK2_MODEL_PATH, ROCK2_TEXTURE_PATH, 0.25f};
        objects.push_back(rock2);

        for (int i = 0; i < ROCK2_NUMBER; ++i)
        {
            std::tie(generatedPosition, generatedScale) = generateRandomRockSpawn(rock2);
            ObjectInstance rockInstance = {Rock, generatedPosition, glm::vec3(0), glm::vec3(generatedScale)};
            objects.back().instances.push_back(rockInstance);
        }

        // Ocean
        Object ocean = {OCEAN_MODEL_PATH, OCEAN_TEXTURE_PATH, 37.0f};
        objects.push_back(ocean);

        ObjectInstance oceanInstance = {Ocean, glm::vec3(-30.0f, -0.13f, -24.0f), glm::vec3(0), glm::vec3(ocean.defaultScale, 3.0, ocean.defaultScale)}; // to avoid to sink, use 5.0 instead of 8.0
        objects.back().instances.push_back(oceanInstance);

        // Text
        Text winText = {WIN_MODEL_PATH, WIN_TEXTURE_PATH, OUT_TEXT_POSITION, glm::vec3(0), glm::vec3(0.2, 0.3, 0)};
        texts.push_back(winText);

        Text loseText = {LOSE_MODEL_PATH, LOSE_TEXTURE_PATH, OUT_TEXT_POSITION, glm::vec3(0), glm::vec3(0.2, 0.3, 0)};
        texts.push_back(loseText);

        Text restartText = {RESTART_MODEL_PATH, RESTART_TEXTURE_PATH, OUT_TEXT_POSITION, glm::vec3(0), glm::vec3(0.08, 0.12, 0)};
        texts.push_back(restartText);

        int i = 0;
        for (const auto obj : objects)
        {
            i += obj.instances.size();
        }

        i += texts.size();

        // Descriptor pool sizes
        uniformBlocksInPool = i + 1;
        texturesInPool = i + 1;
        setsInPool = i + 1;
    }

    // Here you load and setup all your Vulkan objects
    void localInit()
    {
        // Set random seed
        srand(time(NULL));

        // Descriptor Layouts
        descSetLayout.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                                  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

        skyboxDescSetLayout.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                                        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

        textDescSetLayout.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

        // Pipelines
        pipeline.init(this, VERT_SHADER_PATH, FRAG_SHADER_PATH, {&descSetLayout}, VK_COMPARE_OP_LESS, VK_CULL_MODE_BACK_BIT);
        skyboxPipeline.init(this, SKYBOX_VERT_SHADER_PATH, SKYBOX_FRAG_SHADER_PATH, {&skyboxDescSetLayout}, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT);
        textPipeline.init(this, TEXT_VERT_SHADER_PATH, TEXT_FRAG_SHADER_PATH, {&textDescSetLayout}, VK_COMPARE_OP_LESS, VK_CULL_MODE_NONE);

        // Objects
        for (auto &obj : objects)
        {
            obj.init(this);

            for (auto &inst : obj.instances)
            {
                inst.init(this, &descSetLayout, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr, nullptr}, {1, TEXTURE, 0, &obj.texture, nullptr}});
            }
        }

        for (auto &text : texts)
        {
            text.init(this, &textDescSetLayout, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr, nullptr}, {1, TEXTURE, 0, &text.texture, nullptr}});
        }

        skybox.init(this, &skyboxDescSetLayout, {{0, UNIFORM, sizeof(SkyBoxUniformBufferObject), nullptr, nullptr}, {1, SKYBOX, 0, nullptr, &skybox.texture}});

        game.started = true;
    }

    // Here you destroy all the objects you created!
    void localCleanup()
    {
        // Objects
        for (auto &obj : objects)
        {
            obj.cleanup();
        }

        for (auto &text : texts)
        {
            text.cleanup();
        }

        skybox.cleanup();

        // Pipelines
        pipeline.cleanup();
        textPipeline.cleanup();
        skyboxPipeline.cleanup();

        // Descriptor Set Layouts
        descSetLayout.cleanup();
        textDescSetLayout.cleanup();
        skyboxDescSetLayout.cleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

        for (const auto &obj : objects)
        {
            VkBuffer vertexBuffers[] = {obj.model.vertexBuffer};

            // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
            vkCmdBindIndexBuffer(commandBuffer, obj.model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (const auto &inst : obj.instances)
            {
                // property .pipelineLayout of a pipeline contains its layout.
                // property .descriptorSets of a descriptor set contains its elements.
                vkCmdBindDescriptorSets(commandBuffer,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline.pipelineLayout, 0, 1, &inst.descSet.descriptorSets[currentImage],
                                        0, nullptr);

                // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(obj.model.indices.size()), 1, 0, 0, 0);
            }
        }

        // Text
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textPipeline.graphicsPipeline);

        for (const auto &text : texts)
        {
            VkBuffer vertexBuffers[] = {text.model.vertexBuffer};

            // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
            vkCmdBindIndexBuffer(commandBuffer, text.model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            // property .pipelineLayout of a pipeline contains its layout.
            // property .descriptorSets of a descriptor set contains its elements.
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline.pipelineLayout, 0, 1, &text.descSet.descriptorSets[currentImage],
                                    0, nullptr);

            // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(text.model.indices.size()), 1, 0, 0, 0);
        }

        // Skybox
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.graphicsPipeline);

        VkBuffer vertexBuffers[] = {skybox.box.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, skybox.box.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                skyboxPipeline.pipelineLayout, 0, 1,
                                &skybox.descSet.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(skybox.box.indices.size()), 1, 0, 0, 0);
    }

    // Generates random float between min and max
    float getRandFloat(float min, float max)
    {
        return min + (rand() / (RAND_MAX / (max - min)));
    }

    // Generates a random rock position
    glm::vec3 generateRandomRockCoord(bool respawn = false)
    {
        if (respawn)
        {
            return glm::vec3(MIN_X,
                             -0.4f,
                             getRandFloat(MIN_Z, MAX_Z));
        }
        else
        {
            return glm::vec3(getRandFloat(MIN_X, SPAWN_LIMIT_X),
                             -0.4f,
                             getRandFloat(MIN_Z, MAX_Z));
        }
    }

    // Generates position and scale for rock
    std::tuple<glm::vec3, float> generateRandomRockSpawn(Object rock, bool respawn = false)
    {
        bool invalidPosition;
        glm::vec3 position;
        float scale;
        int generation = 0;

        do
        {
            invalidPosition = false;
            position = generateRandomRockCoord(respawn);
            float scaleLimits = rock.defaultScale * 0.4f;
            scale = getRandFloat(rock.defaultScale - scaleLimits, rock.defaultScale + scaleLimits);

            float minX = rock.model.boundaries.minX * scale;
            float maxX = rock.model.boundaries.maxX * scale;

            float minZ = rock.model.boundaries.minZ * scale;
            float maxZ = rock.model.boundaries.maxZ * scale;

            CollisionBox newRockBox = CollisionBox(position, minX, maxX, minZ, maxZ);

            for (const auto &obj : objects)
            {
                for (const auto &inst : obj.instances)
                {
                    if (inst.type == Rock)
                    {
                        CollisionBox rockBox = getCollisionBoxFromInstance(obj, inst);
                        if (newRockBox.checkCollision(rockBox) || glm::length(position - inst.position) < MIN_ROCK_DISTANCE)
                        {
                            invalidPosition = true;
                        }
                    }
                }
            }

            generation++;

        } while (invalidPosition && generation < MAX_POSITION_GENERATION);

        return std::make_tuple(position, scale);
    }

    int getHorizontalDirection()
    {
        if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
        {
            return -1;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    double getDeltaTime()
    {
        static auto lastTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();

        double delta = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        return delta;
    }

    void updateObjectsPositions(double delta, int horDir)
    {
        int pointsGained = 0;
        for (auto &obj : objects)
        {
            for (auto &inst : obj.instances)
            {
                if (inst.type == Boat)
                {
                    inst.rotation.y = 0.0f - horDir * 20.0f;
                }
                else if (inst.type == Rock)
                {
                    inst.position.x += (VERTICAL_SPEED + VERTICAL_SPEED_INCREMENT * game.points) * delta;
                    inst.position.z += horDir * HORIZONTAL_SPEED * delta;

                    // Respawn
                    if (inst.position.x > MAX_X)
                    {
                        float scale;
                        std::tie(inst.position, scale) = generateRandomRockSpawn(obj, true);
                        inst.scale = glm::vec3(scale);
                        pointsGained++;

                        if (game.points + pointsGained >= WIN_POINTS)
                        {
                            game.points += pointsGained;
                            endGame(true);
                        }
                    }
                }

                else if (inst.type == Ocean)
                {
                    inst.position.x += (OCEAN_SPEED + OCEAN_SPEED_INCREMENT) * delta;
                    inst.position.z += (OCEAN_SPEED + OCEAN_SPEED_INCREMENT) * delta;
                }
            }
        }

        game.points += pointsGained;
    }

    void waitRestart()
    {
        if (glfwGetKey(window, GLFW_KEY_SPACE))
        {
            for (auto &obj : objects)
            {
                for (auto &inst : obj.instances)
                {
                    if (inst.type == Boat)
                    {
                        inst.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
                    }
                    else if (inst.type == Rock)
                    {
                        float scale;
                        std::tie(inst.position, scale) = generateRandomRockSpawn(obj);
                        inst.scale = glm::vec3(scale);
                    }
                    else if (inst.type == Ocean)
                    {
                        inst.position = OCEAN_INIT_POS;
                    }
                }
            }
            game.points = 0;
            game.started = true;

            for (auto &text : texts)
            {
                text.position = OUT_TEXT_POSITION;
            }
        }
    }

    CollisionBox getCollisionBoxFromInstance(const Object &object, const ObjectInstance &instance)
    {
        glm::vec2 position = glm::vec2(instance.position.x, instance.position.z);
        float scale = instance.scale.x;

        float minX = object.model.boundaries.minX * scale;
        float maxX = object.model.boundaries.maxX * scale;

        float minZ = object.model.boundaries.minZ * scale;
        float maxZ = object.model.boundaries.maxZ * scale;

        return CollisionBox(position, minX, maxX, minZ, maxZ);
    }

    void checkCollision()
    {
        CollisionBox boatBox = getCollisionBoxFromInstance(objects[0], objects[0].instances[0]);

        for (const auto &obj : objects)
        {
            for (const auto &inst : obj.instances)
            {
                if (inst.type == Rock)
                {
                    CollisionBox rockBox = getCollisionBoxFromInstance(obj, inst);

                    if (boatBox.checkCollision(rockBox))
                    {
                        endGame(false);
                    }
                }
            }
        }
    }

    void endGame(bool win)
    {
        game.highscore = std::max(game.highscore, game.points);

        std::string message;

        if (win)
        {
            message = "You win! You reached " + std::to_string(WIN_POINTS) + " points!";
            texts[0].position = WIN_TEXT_POSITION;
        }
        else
        {
            message = "You lose! You hit a rock!";
            texts[1].position = LOSE_TEXT_POSITION;
        }

        texts[2].position = RESTART_TEXT_POSITION;

        std::cout << std::endl
                  << "==================================" << std::endl
                  << std::endl;
        std::cout << message << std::endl;
        std::cout << "Points: " << game.points << std::endl;
        std::cout << "Highscore: " << game.highscore << std::endl;
        std::cout << "Press SPACEBAR to restart" << std::endl;
        std::cout << std::endl
                  << "==================================" << std::endl
                  << std::endl;

        game.started = false;
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage)
    {
        double delta = getDeltaTime();
        static int horDir = 0;
        void *data;

        if (game.started)
        {
            horDir = getHorizontalDirection();
            updateObjectsPositions(delta, horDir);
            checkCollision();
        }
        else
        {
            waitRestart();
        }

        float aspectRatio = (float)swapChainExtent.width / (float)swapChainExtent.height;

        glm::mat4 camMatrix = glm::lookAt(glm::vec3(4.5f, 0.8f, 0.0f),
                                          glm::vec3(0.0f, 0.6f, 0.0f), 
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projMatrix = glm::perspective(glm::radians(50.0f),
                                                aspectRatio,
                                                NEAR_PLANE, FAR_PLANE);
        projMatrix[1][1] *= -1;

        for (const auto &obj : objects)
        {
            for (const auto &inst : obj.instances)
            {
                UniformBufferObject ubo{};
                ubo.model = glm::translate(glm::mat4(1.0f), inst.position) *
                            glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.y), glm::vec3(0, 1, 0)) *
                            glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.x), glm::vec3(1, 0, 0)) *
                            glm::rotate(glm::mat4(1.0), glm::radians(inst.rotation.z), glm::vec3(0, 0, 1)) *
                            glm::scale(glm::mat4(1.0), inst.scale);
                ubo.view = camMatrix;
                ubo.proj = projMatrix;

                vkMapMemory(device, inst.descSet.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, inst.descSet.uniformBuffersMemory[0][currentImage]);
            }
        }

        // Texts
        for (const auto &text : texts)
        {
            UniformBufferObject ubo{};
            ubo.model = glm::translate(glm::mat4(1.0f), text.position) *
                        glm::rotate(glm::mat4(1.0), glm::radians(text.rotation.y), glm::vec3(0, 1, 0)) *
                        glm::rotate(glm::mat4(1.0), glm::radians(text.rotation.x), glm::vec3(1, 0, 0)) *
                        glm::rotate(glm::mat4(1.0), glm::radians(text.rotation.z), glm::vec3(0, 0, 1)) *
                        glm::scale(glm::mat4(1.0), text.scale);
            ubo.view = glm::mat4(1.0f);
            ubo.proj = glm::mat4(1.0f);

            vkMapMemory(device, text.descSet.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, text.descSet.uniformBuffersMemory[0][currentImage]);
        }

        // Skybox
        SkyBoxUniformBufferObject subo{};
        subo.mMat = glm::mat4(1.0f);
        subo.nMat = glm::mat4(1.0f);
        subo.mvpMat = projMatrix * camMatrix;
        subo.mvpMat = glm::translate(subo.mvpMat, objects[0].instances[0].position);
        subo.mvpMat = glm::scale(subo.mvpMat, glm::vec3(3.0f));

        vkMapMemory(device, skybox.descSet.uniformBuffersMemory[0][currentImage], 0, sizeof(subo), 0, &data);
        memcpy(data, &subo, sizeof(subo));
        vkUnmapMemory(device, skybox.descSet.uniformBuffersMemory[0][currentImage]);
    }
};

// This is the main: probably you do not need to touch this!
int main()
{
    BoatRunner app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
