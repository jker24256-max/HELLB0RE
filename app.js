import * as THREE from 'three';

const $ = (id) => document.getElementById(id);
const universe = $('universe');
const state = { paused:false, speed:0.25, elapsed:0, selected:2, focusTarget:null, toastTimer:null };
let scene, camera, renderer, star, starGlow, raycaster, pointer, clock;
const bodies = [];
const orbitLines = [];
const planetData = [
  {name:'Cinder',type:'Molten rock · Scorched',radius:.27,orbit:3.0,period:8,color:0xe38b70,glow:0x9e4536,phase:1.8},
  {name:'Vesper',type:'Gas dwarf · Cloud bands',radius:.43,orbit:4.25,period:15,color:0xd9b58b,glow:0x96715d,phase:3.4},
  {name:'Aurelia',type:'Terrestrial · Temperate',radius:.36,orbit:5.8,period:24,color:0x819bd9,glow:0x6877cb,phase:0.8},
  {name:'Morrow',type:'Ice world · Frozen ocean',radius:.31,orbit:7.35,period:36,color:0x9bd9e7,glow:0x5aaccb,phase:4.9},
  {name:'Orison',type:'Gas giant · Ringed',radius:.78,orbit:9.45,period:58,color:0xc69fda,glow:0x8a6bb6,phase:2.4}
];
const rand = (()=>{let s=24831;return()=>{s=(s*1664525+1013904223)>>>0;return s/4294967296}})();

function init(){
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0x050716);
  scene.fog = new THREE.FogExp2(0x050716,0.009);
  camera = new THREE.PerspectiveCamera(43,innerWidth/innerHeight,.1,250);
  camera.position.set(0,14,23);
  camera.lookAt(0,0,0);
  renderer = new THREE.WebGLRenderer({antialias:true,alpha:false,powerPreference:'high-performance'});
  renderer.setPixelRatio(Math.min(devicePixelRatio,1.7));
  renderer.setSize(innerWidth,innerHeight);
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  renderer.toneMapping = THREE.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.18;
  renderer.domElement.setAttribute('aria-label','Drag to rotate the star system, scroll to zoom, click a planet to inspect it');
  universe.appendChild(renderer.domElement);
  scene.add(new THREE.AmbientLight(0x7f8bbd,.62));
  const keyLight = new THREE.PointLight(0xffd9ac,150,100,1.4);
  keyLight.position.set(0,0,0); scene.add(keyLight);
  makeStarfield(); makeNebula(); makeSun(); makePlanets();
  raycaster = new THREE.Raycaster(); pointer = new THREE.Vector2(); clock = new THREE.Clock();
  makePlanetList(); selectPlanet(2,false);
  addEventListener('resize',onResize);
  renderer.domElement.addEventListener('pointerdown',onPointerDown);
  renderer.domElement.addEventListener('pointerup',onPointerUp);
  renderer.domElement.addEventListener('pointermove',onPointerMove);
  renderer.domElement.addEventListener('wheel',onWheel,{passive:false});
  $('pauseButton').addEventListener('click',togglePause);
  $('exploreButton').addEventListener('click',()=>{camera.position.set(0,10,18);camera.lookAt(0,0,0);showToast('Welcome to Solace. Choose a world to begin.')});
  $('resetButton').addEventListener('click',resetOrbits);
  $('focusButton').addEventListener('click',focusSelected);
  document.querySelectorAll('.speed-button').forEach(b=>b.addEventListener('click',()=>setSpeed(Number(b.dataset.speed))));
  $('slower').addEventListener('click',()=>setSpeed(Math.max(.25,state.speed/2)));
  $('faster').addEventListener('click',()=>setSpeed(Math.min(12,state.speed*2)));
  animate();
}

function makeStarfield(){
  const count=2600,positions=new Float32Array(count*3),colors=new Float32Array(count*3);
  for(let i=0;i<count;i++){const r=rand()*90+18,a=rand()*Math.PI*2,z=(rand()-.5)*50;positions[i*3]=Math.cos(a)*r;positions[i*3+1]=z;positions[i*3+2]=Math.sin(a)*r;const c=new THREE.Color().setHSL(.56+rand()*.16,.25+rand()*.45,.55+rand()*.45);colors.set([c.r,c.g,c.b],i*3)}
  const g=new THREE.BufferGeometry();g.setAttribute('position',new THREE.BufferAttribute(positions,3));g.setAttribute('color',new THREE.BufferAttribute(colors,3));
  const m=new THREE.PointsMaterial({size:.075,vertexColors:true,transparent:true,opacity:.9,sizeAttenuation:true,depthWrite:false});
  scene.add(new THREE.Points(g,m));
  const farPositions=new Float32Array(900*3);
  for(let i=0;i<900;i++){const r=rand()*140+50,a=rand()*Math.PI*2;farPositions[i*3]=Math.cos(a)*r;farPositions[i*3+1]=(rand()-.5)*90;farPositions[i*3+2]=Math.sin(a)*r}
  const fg=new THREE.BufferGeometry();fg.setAttribute('position',new THREE.BufferAttribute(farPositions,3));
  scene.add(new THREE.Points(fg,new THREE.PointsMaterial({color:0x8e9bc8,size:.045,transparent:true,opacity:.42,depthWrite:false})));
}
function makeNebula(){
  const g=new THREE.BufferGeometry(),p=[],c=[];
  for(let i=0;i<800;i++){const a=rand()*Math.PI*2,r=rand()*30+8;p.push(Math.cos(a)*r,(rand()-.5)*5-3,Math.sin(a)*r);const col=new THREE.Color().setHSL(.68+rand()*.12,.5,.15+rand()*.2);c.push(col.r,col.g,col.b)}
  g.setAttribute('position',new THREE.Float32BufferAttribute(p,3));g.setAttribute('color',new THREE.Float32BufferAttribute(c,3));
  scene.add(new THREE.Points(g,new THREE.PointsMaterial({size:.65,vertexColors:true,transparent:true,opacity:.15,blending:THREE.AdditiveBlending,depthWrite:false})));
}
function makeSun(){
  const geo=new THREE.SphereGeometry(1.12,48,48);
  const mat=new THREE.MeshBasicMaterial({color:0xffd5a0});
  star=new THREE.Mesh(geo,mat);scene.add(star);
  starGlow=new THREE.Sprite(new THREE.SpriteMaterial({map:glowTexture(),color:0xffb86c,transparent:true,blending:THREE.AdditiveBlending,depthWrite:false}));
  starGlow.scale.set(7.5,7.5,1);scene.add(starGlow);
  const corona=new THREE.Mesh(new THREE.SphereGeometry(1.24,32,32),new THREE.MeshBasicMaterial({color:0xffa75d,transparent:true,opacity:.12,side:THREE.BackSide}));
  star.add(corona);
}
function glowTexture(){
  const canvas=document.createElement('canvas');canvas.width=128;canvas.height=128;const ctx=canvas.getContext('2d');
  const gr=ctx.createRadialGradient(64,64,2,64,64,64);gr.addColorStop(0,'rgba(255,225,175,1)');gr.addColorStop(.15,'rgba(255,170,95,.62)');gr.addColorStop(.42,'rgba(255,116,75,.17)');gr.addColorStop(1,'rgba(255,90,60,0)');ctx.fillStyle=gr;ctx.fillRect(0,0,128,128);
  const tex=new THREE.CanvasTexture(canvas);tex.colorSpace=THREE.SRGBColorSpace;return tex;
}
function makePlanets(){
  planetData.forEach((d,i)=>{
    const pivot=new THREE.Group();scene.add(pivot);
    const geo=new THREE.SphereGeometry(d.radius,40,32);
    const mat=new THREE.MeshStandardMaterial({color:d.color,roughness:.82,metalness:.02});
    const mesh=new THREE.Mesh(geo,mat);mesh.position.x=d.orbit;pivot.add(mesh);mesh.userData.planetIndex=i;
    const glow=new THREE.Sprite(new THREE.SpriteMaterial({map:glowTexture(),color:d.glow,transparent:true,opacity:.24,blending:THREE.AdditiveBlending,depthWrite:false}));
    glow.scale.set(d.radius*5,d.radius*5,1);glow.position.copy(mesh.position);pivot.add(glow);
    if(i===2){
      const atmo=new THREE.Mesh(new THREE.SphereGeometry(d.radius*1.12,32,24),new THREE.MeshBasicMaterial({color:0x75b9ff,transparent:true,opacity:.14,side:THREE.BackSide}));
      mesh.add(atmo);
      const cloud=new THREE.Mesh(new THREE.SphereGeometry(d.radius*1.015,32,24),new THREE.MeshStandardMaterial({color:0xc4e4ec,transparent:true,opacity:.2,roughness:1}));
      mesh.add(cloud);
    }
    if(i===4){
      const ring=new THREE.Mesh(new THREE.RingGeometry(d.radius*1.35,d.radius*2.05,64),new THREE.MeshStandardMaterial({color:0xc7b2dd,side:THREE.DoubleSide,transparent:true,opacity:.7,roughness:.95}));
      ring.rotation.x=Math.PI/2.45;mesh.add(ring);
      const ring2=new THREE.Mesh(new THREE.RingGeometry(d.radius*2.12,d.radius*2.25,64),new THREE.MeshStandardMaterial({color:0x7f82bd,side:THREE.DoubleSide,transparent:true,opacity:.3}));
      ring2.rotation.x=Math.PI/2.45;mesh.add(ring2);
    }
    const orbitGeo=new THREE.BufferGeometry(),pts=[];
    for(let j=0;j<=180;j++){const a=j/180*Math.PI*2;pts.push(Math.cos(a)*d.orbit,0,Math.sin(a)*d.orbit)}
    orbitGeo.setFromPoints(pts.map(p=>new THREE.Vector3(...p)));
    const orbit=new THREE.LineLoop(orbitGeo,new THREE.LineBasicMaterial({color:i===2?0x7e80cb:0x4e5c85,transparent:true,opacity:i===2?.38:.2}));
    scene.add(orbit);orbitLines.push(orbit);
    bodies.push({data:d,pivot,mesh,glow,phase:d.phase,cloud:mesh.children.find(x=>x.material?.transparent&&x.geometry?.type==='SphereGeometry')});
  });
}
function makePlanetList(){
  const list=$('planetList');list.innerHTML='';
  planetData.forEach((p,i)=>{const b=document.createElement('button');b.className='planet-chip';b.textContent=p.name;b.style.setProperty('--planet-color','#'+p.color.toString(16).padStart(6,'0'));b.addEventListener('click',()=>selectPlanet(i,true));list.appendChild(b)});
}
function selectPlanet(i,notify=true){
  state.selected=i;const d=planetData[i];
  $('selectedName').textContent=d.name;$('selectedType').textContent=d.type;
  $('planetThumb').style.background='radial-gradient(circle at 32% 28%, #fff, #'+d.color.toString(16).padStart(6,'0')+' 35%, #10152d 100%)';
  document.querySelectorAll('.planet-chip').forEach((b,j)=>b.classList.toggle('active',j===i));
  if(notify)showToast(d.name+' selected. Focus to inspect its orbit.');
}
function focusSelected(){
  const b=bodies[state.selected];const world=b.mesh.getWorldPosition(new THREE.Vector3());
  state.focusTarget=world.clone();
  const direction=new THREE.Vector3().subVectors(camera.position,world).normalize();
  camera.position.copy(world).add(direction.multiplyScalar(Math.max(3.2,b.data.radius*8))).add(new THREE.Vector3(0,1.4,0));
  camera.lookAt(world);showToast('Focusing on '+b.data.name+'.');
}
function resetOrbits(){state.elapsed=0;state.focusTarget=null;camera.position.set(0,14,23);camera.lookAt(0,0,0);bodies.forEach(b=>{b.phase=b.data.phase;b.pivot.rotation.y=0});showToast('Orbital positions reset.')}
function setSpeed(v){state.speed=v;document.querySelectorAll('.speed-button').forEach(b=>b.classList.toggle('active',Number(b.dataset.speed)===v));$('speedReadout').textContent=v+'×'}
function togglePause(){state.paused=!state.paused;$('pauseButton').textContent=state.paused?'▶':'Ⅱ';$('pauseButton').setAttribute('aria-label',state.paused?'Resume simulation':'Pause simulation');$('runStatus').textContent=state.paused?'PAUSED':'RUNNING';showToast(state.paused?'Simulation paused.':'Simulation resumed.')}
function showToast(text){const t=$('toast');t.textContent=text;t.classList.add('show');clearTimeout(state.toastTimer);state.toastTimer=setTimeout(()=>t.classList.remove('show'),2400)}
function onResize(){camera.aspect=innerWidth/innerHeight;camera.updateProjectionMatrix();renderer.setSize(innerWidth,innerHeight);renderer.setPixelRatio(Math.min(devicePixelRatio,1.7))}
let down=null,dragging=false,lastX=0,lastY=0;
function onPointerDown(e){down={x:e.clientX,y:e.clientY};lastX=e.clientX;lastY=e.clientY;dragging=false}
function onPointerMove(e){if(!down)return;const dx=e.clientX-lastX,dy=e.clientY-lastY;if(Math.hypot(e.clientX-down.x,e.clientY-down.y)>4)dragging=true;if(dragging){const offset=camera.position.clone();const spherical=new THREE.Spherical().setFromVector3(offset);spherical.theta-=dx*.006;spherical.phi=THREE.MathUtils.clamp(spherical.phi+dy*.006,.2,Math.PI-.2);spherical.radius=THREE.MathUtils.clamp(spherical.radius,7,52);camera.position.setFromSpherical(spherical);camera.lookAt(0,0,0)}lastX=e.clientX;lastY=e.clientY}
function onPointerUp(e){if(!dragging&&down){const rect=renderer.domElement.getBoundingClientRect();pointer.x=((e.clientX-rect.left)/rect.width)*2-1;pointer.y=-((e.clientY-rect.top)/rect.height)*2+1;raycaster.setFromCamera(pointer,camera);const hits=raycaster.intersectObjects(bodies.map(b=>b.mesh),false);if(hits.length)selectPlanet(hits[0].object.userData.planetIndex,true)}down=null}
function onWheel(e){e.preventDefault();const s=new THREE.Spherical().setFromVector3(camera.position);s.radius=THREE.MathUtils.clamp(s.radius+e.deltaY*.012,7,52);camera.position.setFromSpherical(s);camera.lookAt(0,0,0)}
function animate(){
  requestAnimationFrame(animate);
  const dt=Math.min(clock.getDelta(),.05);
  if(!state.paused){
    state.elapsed+=dt*state.speed;
    bodies.forEach((b,i)=>{const angle=b.phase+state.elapsed/(b.data.period*1.6);b.pivot.rotation.y=angle;b.mesh.rotation.y+=dt*(i%2?-.22:.32);if(b.cloud)b.cloud.rotation.y+=dt*.12});
    star.scale.setScalar(1+Math.sin(state.elapsed*1.3)*.012);starGlow.material.rotation+=dt*.015;
  }
  const secs=Math.floor(state.elapsed),h=String(Math.floor(secs/3600)).padStart(2,'0'),m=String(Math.floor(secs%3600/60)).padStart(2,'0'),s=String(secs%60).padStart(2,'0');
  $('clock').textContent='T+ '+h+':'+m+':'+s;
  renderer.render(scene,camera);
}
init();