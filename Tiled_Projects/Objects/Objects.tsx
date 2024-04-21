<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.2" name="objects" tilewidth="1480" tileheight="988" tilecount="6" columns="0">
 <grid orientation="orthogonal" width="1" height="1"/>
 <tile id="0">
  <image width="200" height="300" source="../../game_engine/resources/images/big_box.png"/>
 </tile>
 <tile id="1">
  <properties>
   <property name="1" value="&quot;type&quot; : &quot;Rigidbody&quot;"/>
   <property name="2" value="&quot;type&quot; : &quot;SpriteRenderer&quot;, &quot;sprite&quot; : &quot;big_circle&quot;"/>
  </properties>
  <image width="200" height="200" source="../../game_engine/resources/images/big_circle.png"/>
 </tile>
 <tile id="2">
  <properties>
   <property name="1" value="&quot;type&quot; : &quot;Rigidbody&quot;, &quot;body_type&quot; : &quot;static&quot;"/>
   <property name="2" value="&quot;type&quot; : &quot;SpriteRenderer&quot;, &quot;sprite&quot; : &quot;box1&quot;"/>
  </properties>
  <image width="100" height="100" source="../../game_engine/resources/images/box1.png"/>
 </tile>
 <tile id="3">
  <image width="100" height="100" source="../../game_engine/resources/images/box2.png"/>
 </tile>
 <tile id="4">
  <properties>
   <property name="1" value="&quot;type&quot; : &quot;Rigidbody&quot;"/>
   <property name="2" value="&quot;type&quot; : &quot;SpriteRenderer&quot;, &quot;sprite&quot; : &quot;circle&quot;"/>
  </properties>
  <image width="100" height="100" source="../../game_engine/resources/images/circle.png"/>
 </tile>
 <tile id="5">
  <properties>
   <property name="1" value="&quot;type&quot; : &quot;Rigidbody&quot;, &quot;body_type&quot; : &quot;static&quot;, &quot;has_trigger&quot; : true, &quot;has_collider&quot; : false"/>
   <property name="2" value="&quot;type&quot; : &quot;SpriteRenderer&quot;, &quot;sprite&quot; : &quot;sky&quot;"/>
  </properties>
  <image width="1480" height="988" source="../../game_engine/resources/images/sky.png"/>
 </tile>
</tileset>
