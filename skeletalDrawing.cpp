

#include "skeletalDrawing.h"
#include "renderer.h"
#include "boneActor.h"
#include "skeletalActor.h"
#include "spriteMeshLoader.h"
#include "input.h"

SkeletalDrawing::SkeletalDrawing(){


drawType=DRAW_PARTICLES;
skeletonName="NULL";
registerProperties();
}

SkeletalDrawing::~SkeletalDrawing(){}

void SkeletalDrawing::registerProperties(){

createMemberID("SKELETONNAME",&skeletonName,this);
ParticleSystem::registerProperties();
}

void SkeletalDrawing::setup(){

ParticleSystem::setup();
}

void SkeletalDrawing::updateShaders(){

    ParticleSystem::updateShaders();


    if (sceneShaderID=="skinning"){

        GLint uniform_location=0;

        int boneIndices[]={-1,-1,-1,-1};
        int current=0;

        //determine the bones we've got selected
        for (int i=0;i<(int)input->selectedActors.size();i++){
            for (int j=0;j<(int)bones.size();j++){
                if (input->selectedActors[i]==bones[j]){
                    if (current<4){
                        boneIndices[current]=j;
                        current++;
                    }else cout << "too many bones selected!" << endl;
                }
            }
        }

        uniform_location = glGetUniformLocation(renderer->shaderList[sceneShaderID]->shader, "boneIndices");
        glUniform1iv(uniform_location,4,(GLint*)&boneIndices);
    }

}

void SkeletalDrawing::trigger(Actor * other){

}



void SkeletalDrawing::create(){renderer->addActor(this);}
