/**
 * Loading Images. 
 * 
 * Processing applications can load images from the network. 
 * 
 */

PImage img;
float roll, pitch,yaw;

void setup() {
  size (1000, 700, P3D);
  frameRate(30);
}

void draw() {
  background(0);
   translate(width/2, height/2, 0);
  background(233);
  textSize(22);
  text("Roll: " + int(roll) + "     Pitch: " + int(pitch), -100, 265);
  // Rotate the object
  rotateX(radians(-pitch));
  rotateZ(radians(roll));
  rotateY(radians(yaw));
  
  // 3D 0bject
  textSize(30);  
  fill(0, 76, 153);
  box (386, 40, 200); // Draw box
  textSize(25);
  fill(255, 255, 255);
  text("www.HowToMechatronics.com", -183, 10, 101);

String lines[] = loadStrings("http://192.168.4.1");
//String lineS = lines.toString();

//println(split(lines[0], ',')[0]);
println(frameRate);
//println("there are " + lines.length + " lines");
//for (int i = 0 ; i < lines.length; i++) {
//  println(lines[i]);
//}
//    String items[] = split(lines, ',');
//    if (items.length > 1) {
//      //--- Roll,Pitch in degrees
      roll = float(split(lines[0], ',')[0]);
      pitch = float(split(lines[0], ',')[1]);
      yaw = float(split(lines[0], ',')[2]);
      //roll = float(split(lines[0], ',')[3]);
      //pitch = float(split(lines[0], ',')[4]);
      //yaw = float(split(lines[0], ',')[5]);


    //}
}
