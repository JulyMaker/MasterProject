#ifndef SCENE5_H
#define SCENE5_H

#include <iostream>
#include <time.h>
#include <vector>

using namespace std;

class Scene5 : public Test {
	public:
		Scene5(){		
			m_world->SetGravity(b2Vec2(0, -10));
			// Configure particle system parameters.
			//m_particleSystem->SetRadius(100.0f);
			//m_particleSystem->SetMaxParticleCount(100000);
			//m_particleSystem->SetDestructionByAge(true);

			TestMain::GetFilesNames(geomFile_, neuronFile_);
				
			vector<b2Vec2> geomRead;
			vector<b2Vec2> particiones;
			vector<float>  times;		

			// { Read Geom and Partition File}
			if (geomFile_ != ""){
				FILE *archivo;
				archivo = fopen(geomFile_.c_str(), "r");

				float x, y;
				int maxParticles;
				unsigned int r, g, b, a;
				unsigned int vertices, partitions, emiters;

				fscanf(archivo, "%u", &vertices);
				fscanf(archivo, "%u", &partitions);
				fscanf(archivo, "%u", &emiters);

				// Geom
				for (unsigned int cont = 0; cont < vertices; ++cont)
				{		
					fscanf(archivo, "%f %f", &x, &y);
					maxX = max(x, maxX);
					minX = min(x, minX);
					maxY = max(y, maxY);
					minY = min(y, minY);
					geomRead.push_back(b2Vec2(x, y));
				}
				Zoom = max((maxX - minX), (maxY - minY))/2;
				// Partition
				for (unsigned int cont = 0; cont < partitions; ++cont){
					fscanf(archivo, "%f %f", &x, &y);
					particiones.push_back(b2Vec2(x, y));
				}
				
				if (emiters > 0){
					fscanf(archivo, "%f %d", &x, &maxParticles);

					m_particleSystem->SetRadius(x);
					m_particleSystem->SetMaxParticleCount(maxParticles);
					m_particleSystem->SetDestructionByAge(true);

					const float32 faucetLength = m_particleSystem->GetRadius() * 2.0f * k_faucetLength;

					for (unsigned int cont = 0; cont < emiters; ++cont){
						RadialEmitter rEmit;

						rEmit.SetParticleSystem(m_particleSystem);
						fscanf(archivo, "%f %f", &x, &y);
						rEmit.SetPosition(b2Vec2(x, y));
						fscanf(archivo, "%f %f", &x, &y);
						rEmit.SetVelocity(b2Vec2(x, y));
						rEmit.SetSize(b2Vec2(0.0f, faucetLength));
						fscanf(archivo, "%u %u %u %u", &r, &g, &b, &a);
						rEmit.SetColor(b2ParticleColor((uint8)r, (uint8)g, (uint8)b, (uint8)a));
						fscanf(archivo, "%f", &x);
						rEmit.SetEmitRate(x);
						rEmit.SetParticleFlags(TestMain::GetParticleParameterValue());

						m_emitters.push_back(rEmit);
					}
				}
				fclose(archivo);
			
				try {
					if (geomRead.size() < 2) throw (int)geomRead.size();
					// Vertex Geom
					b2BodyDef bd;
					b2Body* ground = m_world->CreateBody(&bd);
					b2ChainShape bodyGeom;

					bodyGeom.CreateLoop(&geomRead[0], geomRead.size());

					b2FixtureDef defB;
					defB.shape = &bodyGeom;
					ground->CreateFixture(&defB);

					if (particiones.size() < 2) throw (int) particiones.size();
					// Partition Vertex
					b2BodyDef bdP;
					b2Body* groundP = m_world->CreateBody(&bdP);
					b2ChainShape partitionGeom;
					
					partitionGeom.CreateChain(&particiones[0], particiones.size());

					b2FixtureDef defP;
					defP.shape = &partitionGeom;
					groundP->CreateFixture(&defP);
				}
				catch (int error) {
					cerr << error <<" punto. Debe contener al menos 2 puntos"<<endl;
				}				
			}
			// { Read Neuron File}
			if (neuronFile_ != ""){
				//FILE *archivo;
				//archivo = fopen(neuronFile_.c_str(), "r");
				//
				//float tiempo;
				//
				//while (fscanf(archivo, "%f", &tiempo) != EOF)
				//{
				//	time.push_back(tiempo);
				//}
			}
			// Don't restart the test when changing particle types.
			TestMain::SetRestartOnParticleParameterChange(false);
			// Limit the set of particle types.
			TestMain::SetParticleParameters(k_paramDef, k_paramDefCount);
			// Create the particles.
			//ResetParticles();			
			time(&initHour);
		}		

		void setFiles(const string geomFile, const string neuronFile){ geomFile_ = geomFile; neuronFile_ = neuronFile; }
		
		b2ParticleColor colorPorcentaje(int p){
			int intervalo = (p>0) + (p>20) + (p>40) + (p>60) + (p>80);
			switch (intervalo){
			case 0: return b2ParticleColor(0, 0, 0, 0); // Transparente
			case 1:	return b2ParticleColor(0, 255, 0, 255); // green
			case 2: return b2ParticleColor(255, 0, 255, 255);
			case 3: return b2ParticleColor(0, 0, 255, 255); // blue
			case 4: return b2ParticleColor(255, 255, 0, 255);
			case 5: return b2ParticleColor(255, 0, 0, 255); //red
			default:
				return b2ParticleColor(255, 255, 255, 255);
				break;
			}
		}

		virtual void Step(Settings* settings){
			Test::Step(settings);		

			const float32 dt = 1.0f / settings->hz;		
			m_particleColorOffset += dt;
			// Keep m_particleColorOffset in the range 0.0f..k_ParticleColorsCount.
			if (m_particleColorOffset >= (float32)k_ParticleColorsCount){ m_particleColorOffset -= (float32)k_ParticleColorsCount; }
			// Propagate the currently selected particle flags.
			for (unsigned int i = 0; i < m_emitters.size(); ++i){
				m_emitters.at(i).SetParticleFlags(TestMain::GetParticleParameterValue());
				//m_emitters.at(i).SetColor(colorPorcentaje(3));
				m_emitters.at(i).Step(dt, NULL, 0); // Create the particles.
			}

			time_t currentTime;
			time(&currentTime);

			double seg = difftime(currentTime, initHour);
			
			if (seg > 10){
				m_emitters.at(2).SetPosition(b2Vec2(1000.0f, -600.0f));	
				
			}
			m_debugDraw.DrawString(700, 60, "Time { %02u : %02u }", (unsigned int)seg / 60, (unsigned int)seg % 60);
			//m_debugDraw.DrawString(700, 75, "Barrier Pos: %f", m_position);
			//m_debugDraw.DrawString(700, 90, "Num Particles2: %i", bottom2);
			//m_debugDraw.DrawString(700, 105, "Barrier Pos2: %f", m_position2);
			//m_debugDraw.DrawString(700, 120, "Num Particles3: %i", bottom3);
			//m_debugDraw.DrawString(700, 135, "Barrier Pos3: %f", m_position3);
		}

		// Determine whether a point is in the container.
		bool InContainer(const b2Vec2& p) const{ return p.x >= -k_containerHalfWidth && p.x <= k_containerHalfWidth && p.y >= 0.0f && p.y <= k_containerHalfHeight * 2.0f; }
		float32 GetDefaultViewZoom() const{ return (Zoom <= 1) ? pow(Zoom, 2) : sqrt(Zoom);}
		void GetCam(float &w, float &h) const{ w = (maxX + minX)/2; h = (maxY + minY)/2; }
		static Test* Create(){ return new Scene5; }

	private:
		float32 m_particleColorOffset;
		vector<RadialEmitter> m_emitters;
		int32 porcentaje;
		int32 porcentaje2;
		int32 porcentaje3;
		int32 porcentaje4;

		float32 m_position;
		//float32 m_position2;
		//float32 m_position3;

		b2Body* m_barrierBody;
		//b2Body* m_barrierBody2;
		//b2Body* m_barrierBody3;
		b2ParticleGroup* m_particleGroup;

		float maxX = -FLT_MAX, minX = FLT_MAX, maxY = -FLT_MAX, minY = FLT_MAX;

		time_t initHour;

	private:
		static const int32 k_maxParticleCount;
		static const ParticleParameter::Value k_paramValues[];
		static const ParticleParameter::Definition k_paramDef[];
		static const uint32 k_paramDefCount;

		static const float32 k_containerWidth;
		static const float32 k_containerHeight;
		static const float32 k_containerHalfWidth;
		static const float32 k_containerHalfHeight;
		static const float32 k_barrierHeight;
		static const float32 k_barrierMovementIncrement;

		static const float32 k_faucetLength;
		static const float32 k_faucetWidth;
		static const float32 k_faucetHeight;
};

const float32 Scene5::k_faucetLength = 2.0f;
const float32 Scene5::k_faucetWidth = 0.1f;
const float32 Scene5::k_faucetHeight = 15.0f;

const float32 Scene5::k_containerWidth = 2.0f;
const float32 Scene5::k_containerHeight = 5.0f;

const float32 Scene5::k_containerHalfWidth = Scene5::k_containerWidth / 2.0f;
const float32 Scene5::k_containerHalfHeight = Scene5::k_containerHeight / 2.0f;
const float32 Scene5::k_barrierHeight = Scene5::k_containerHalfHeight / 100.0f;
const float32 Scene5::k_barrierMovementIncrement = Scene5::k_containerHalfHeight * 0.1f;


const int32 Scene5::k_maxParticleCount = 10000;
const ParticleParameter::Value Scene5::k_paramValues[] = {
	{ b2_waterParticle, ParticleParameter::k_DefaultOptions, "water" },
	{ b2_waterParticle, ParticleParameter::k_DefaultOptions | ParticleParameter::OptionStrictContacts, "water (strict)" },
	{ b2_viscousParticle, ParticleParameter::k_DefaultOptions, "viscous" },
	{ b2_powderParticle, ParticleParameter::k_DefaultOptions, "powder" },
	{ b2_tensileParticle, ParticleParameter::k_DefaultOptions, "tensile" },
	{ b2_colorMixingParticle, ParticleParameter::k_DefaultOptions, "color mixing" },
	{ b2_staticPressureParticle, ParticleParameter::k_DefaultOptions, "static pressure" }, };

const ParticleParameter::Definition Scene5::k_paramDef[] = { { Scene5::k_paramValues, B2_ARRAY_SIZE(Scene5::k_paramValues) }, };
const uint32 Scene5::k_paramDefCount = B2_ARRAY_SIZE(Scene5::k_paramDef);

#endif