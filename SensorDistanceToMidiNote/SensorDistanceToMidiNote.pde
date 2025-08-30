/**
 * Processing 4: Serial distance → MIDI Note On
 * - Expects Serial lines like: "Distance: 42 cm"
 * - Maps 10–100 cm to MIDI notes 84..36 (closer = higher pitch)
 * - Sends Note On to default MIDI device; turns previous note off
 *
 * Requires:
 *   - processing.serial.* (built-in)
 *   - javax.sound.midi.* (built-in in Java)
 *
 * Make sure your Arduino is printing a single distance per line.
 * Baud rate must match your Arduino sketch (9600 here).
 */

import processing.serial.*;
import javax.sound.midi.*;
import java.util.regex.*;

Serial port;
Receiver midiOut;

final int MIDI_CHANNEL = 0;       // 0-15
final int NOTE_MIN = 23;          // 12
final int NOTE_MAX = 96;          // C7
final float CM_NEAR = 2;         // nearest distance to consider
final float CM_FAR  = 100;        // farthest distance to consider

int lastNote = -1;
long lastDataMs = 0;
final int SILENCE_TIMEOUT_MS = 1000; // send Note Off if no data

Pattern numberPattern = Pattern.compile("([-+]?[0-9]*\\.?[0-9]+)");

void setup() {
  size(640, 240);
  background(0);
  fill(255);
  textAlign(LEFT, TOP);
  textSize(14);

  println("Available serial ports:");
  //println(Serial.list());

  // Open COM3 @ 9600 (adjust if needed)
  try {
    port = new Serial(this, "COM3", 9600);
    port.bufferUntil('\n');
    println("Opened serial: COM3 @ 9600");
  } catch (Exception e) {
    println("Failed to open COM3: " + e.getMessage());
  }

  // Open default MIDI Receiver (usually the default synth or selected MIDI out)
  listMidiDevices();
  //try {
    midiOut = openMidiOutDevice(2);
    println("MIDI receiver opened: " + midiOut);
  //} catch (MidiUnavailableException e) {
  //  println("No default MIDI receiver available: " + e.getMessage());
  //}
  

}


void draw() {
  background(0);
  String status = "Serial: " + (port != null ? "OK" : "NOT OPEN")
                + "   |   MIDI: " + (midiOut != null ? "OK" : "NOT OPEN");
  text(status, 10, 10);

  // If no data recently, turn off any held note
  if (millis() - lastDataMs > SILENCE_TIMEOUT_MS && lastNote >= 0) {
    noteOff(lastNote, 0);
    lastNote = -1;
  }
}


// Opens a MIDI Out device by index from listMidiDevices()
// Returns the Receiver you can use to send Note On/Off, etc.
Receiver openMidiOutDevice(int deviceIndex) {
  try {
    MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
    if (deviceIndex < 0 || deviceIndex >= infos.length) {
      println("Invalid device index: " + deviceIndex);
      return null;
    }

    MidiDevice device = MidiSystem.getMidiDevice(infos[deviceIndex]);

    // Ensure the device can receive MIDI messages
    if (device.getMaxReceivers() == 0) {
      println("Device " + infos[deviceIndex].getName() + " is not a MIDI Out.");
      return null;
    }

    // Open the device if not already open
    if (!device.isOpen()) {
      device.open();
    }

    println("Opened MIDI Out: " + infos[deviceIndex].getName());
    return device.getReceiver();

  } catch (MidiUnavailableException e) {
    println("MIDI device unavailable: " + e.getMessage());
    return null;
  }
}

void serialEvent(Serial p) {
  String line = p.readStringUntil('\n');
  if (line == null) return;
  line = line.trim();
  lastDataMs = millis();

  // Extract first number in the line (works for "Distance: 42 cm" or just "42")
  Float cmVal = parseFirstFloat(line);
  if (cmVal == null || cmVal.isNaN() || cmVal.isInfinite()) return;

  // Constrain and map distance to note/velocity
  float cm = constrain(cmVal, CM_NEAR, CM_FAR);

  // Closer → higher pitch
  int note = (int)map(cm, CM_NEAR, CM_FAR, NOTE_MAX, NOTE_MIN);
  note = constrain(note, NOTE_MIN, NOTE_MAX);

  // Velocity: nearer → louder
  int velocity = (int)map(cm, CM_NEAR, CM_FAR, 120, 30);
  velocity = constrain(velocity, 127, 60);

  // If the note changed, send Note Off for the previous, then Note On for new
  if (note != lastNote) {
    if (lastNote >= 0) noteOff(lastNote, 0);
    noteOn(note, velocity);
    lastNote = note;
  }

  // Debug text
  println("cm=" + nf(cm, 0, 1) + "  note=" + note + "  vel=" + velocity);
}

Float parseFirstFloat(String s) {
  Matcher m = numberPattern.matcher(s);
  if (m.find()) {
    try {
      return Float.parseFloat(m.group(1));
    } catch (Exception e) {
      return null;
    }
  }
  return null;
}

void noteOn(int note, int velocity) {
  if (midiOut == null) return;
  try {
    ShortMessage msg = new ShortMessage();
    msg.setMessage(ShortMessage.NOTE_ON, MIDI_CHANNEL, note, velocity);
    midiOut.send(msg, -1);
  } catch (Exception e) {
    println("noteOn error: " + e.getMessage());
  }
}

void noteOff(int note, int velocity) {
  if (midiOut == null) return;
  try {
    ShortMessage msg = new ShortMessage();
    msg.setMessage(ShortMessage.NOTE_OFF, MIDI_CHANNEL, note, velocity);
    midiOut.send(msg, -1);
  } catch (Exception e) {
    println("noteOff error: " + e.getMessage());
  }
}

void stop() {
  // Turn off any lingering note on exit
  if (lastNote >= 0) noteOff(lastNote, 0);

  if (midiOut != null) {
    // Many Receivers don't need explicit close, but do it if supported
    try {
      MidiDevice dev = (MidiDevice) ((Receiver) midiOut).getClass()
                         .getDeclaredField("this$0").get(midiOut);
      if (dev != null && dev.isOpen()) dev.close();
    } catch (Exception ignored) {}
  }
  super.stop();
}

void listMidiDevices() {
  MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
  println("=== Available MIDI Devices ===");
  for (int i = 0; i < infos.length; i++) {
    try {
      MidiDevice device = MidiSystem.getMidiDevice(infos[i]);
      String type = device.getMaxReceivers() != 0 ? "OUT" : "IN";
      println(i + ": " + infos[i].getName() + " — " 
                + infos[i].getDescription() + " [" + type + "]");
    } catch (MidiUnavailableException e) {
      println(i + ": " + infos[i].getName() + " (Unavailable)");
    }
  }
  println("==============================");
}
