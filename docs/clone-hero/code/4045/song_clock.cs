// Clone Hero v1.0.0.4045 (Mono build), decompiled with ilspycmd 11.0.0.9375 (ilspycmd -p -o <empty folder> <dll>) from
// C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll on 2026-11-23.
// Only the relevant lines are kept, copied unchanged. Names are the game's scrambled names.
// Line numbers in each banner are lines of the per-class files that command writes, not of this file.

// ---- C1 the deadline's caller passes the game manager's song time, DrumsPlayer.ʸʸʳʽʲʻʿʸʳʸʽ : DrumsPlayer.cs lines 872 to 875 of the ilspycmd output ----
		if (ʹˀʼʵʳˀʻʼʵʿʵ < 0.5f && ʹˀʼʵʳˀʻʼʵʿʵ + ʴʷˀʳʿʴʿʹʴʾʸ >= 0.5f && !ˁʸʴʵʳʾʽʿʹʷʿ)
		{
			ʻˀʻʿʷʷʾʿˁʵʽ(ˀʹʳʴʸʺʳʴʷʵʽ.ʲʻʺʹʺʲʷʾʻʺʹ);
		}

// ---- C2 the song time field ʲʻʺʹʺʲʷʾʻʺʹ on GameManager : GameManager.cs lines 127 to 127 of the ilspycmd output ----
	public double ʲʻʺʹʺʲʷʾʻʺʹ;

// ---- C3 song load (coroutine ʸʻˀʲʿʳʾʿʿʾʴ): the clock's divisor is set to the song speed, then song time is read from the clock : GameManager.cs lines 2229 to 2236 of the ilspycmd output ----
		ʿʻʽʼʹʸˀʶʲʶʲ = ʲʶʶˁʲʲʴʶʴˀʻ();
		ʼˁʸʹʽʺˀʺˁʾʻ.ʸʲʿʽʴʴʹʴʶʵʼ = -1.0;
		ʼˁʸʹʽʺˀʺˁʾʻ.ˁʵʴʾʲʿʿʾʹʶʾ = ʿʻʽʼʹʸˀʶʲʶʲ;
		ʼˁʸʹʽʺˀʺˁʾʻ.ʸˁʳʿʽʶʼʾʵˀˁ = ʵʲʵʲʷʴʶʿʾʶʻ;
		ʼˁʸʹʽʺˀʺˁʾʻ.ʸʾˁʾʽʾʷʼʲˀʴ = ʻʳʴʵʺʳʾˁʷʷʿ;
		ʼˁʸʹʽʺˀʺˁʾʻ.ʽʸʿʹʾʹʼʹʶʽʹ = ʹʺʽˁʽˁˀʼʶʷʼ.ʳʷʴʺʳʹʷʾʲʷʾ.GetFloatPercent;
		ʺʿʵʴʷʸʳʶʷʾʻ = (ʲʻʺʹʺʲʷʾʻʺʹ = ʼˁʸʹʽʺˀʺˁʾʻ.ˁʶʾʹʽʾʼʳʷʺʽ);
		ʷʴʺʵʵʸʾʵʳʵʻ = ʼˁʸʹʽʺˀʺˁʾʻ.ˁʷʹʷʴʳʾʲʼʸʻ;

// ---- C4 GameManager.Update: last frame's chart tick is kept, the clock ticks, song time is read from it, and this frame's chart tick is made from song time : GameManager.cs lines 2292 to 2312 of the ilspycmd output ----
		ʻʴʶʼʽʵˁʳʼʳʺ = ʺʼʲʾʷˀʽʷʲʹʿ;
		ˀʴʵʴʺʲʻʻʻʸʸ = false;
		if (ˀˀʺʿʳʺʵʿʹʻʵ && !CHNetManager.ʷʲʺʷʻʳʾʶˁˀʷ.ʴʺʸʶʻʵʲʸʶʻʻ)
		{
			return;
		}
		float num = Mathf.Round(1f / Time.unscaledDeltaTime);
		if (200f / num < 1f)
		{
			ʿʸʺʻʼʵʾʷˁʺʼ.ʻʵʻʶʿʳʵʵʶʿʷ += 200f / num;
		}
		ʿʸʺʻʼʵʾʷˁʺʼ.ʴʵʾˀʾʺˀˁʽʵʹ();
		ʼˁʸʹʽʺˀʺˁʾʻ.ʿʸʶˁʷˁʼʳʶʴʸ();
		ʲʻʺʹʺʲʷʾʻʺʹ = ʼˁʸʹʽʺˀʺˁʾʻ.ˁʶʾʹʽʾʼʳʷʺʽ;
		ʷʴʺʵʵʸʾʵʳʵʻ = ʼˁʸʹʽʺˀʺˁʾʻ.ˁʷʹʷʴʳʾʲʼʸʻ;
		ʳʻʾʿʺʼʵˀʻʾʽ.ʹʼʻˀʲʴʻʺʵʾʼ();
		ʿʻʷʳʶʽʼʹˀˁʻ();
		ʵʹˁˀʷʽʾʲˁʲʶ.ˀʶʽʾʾʻʴʴʷʶʼ(ʲʻʺʹʺʲʷʾʻʺʹ);
		ˀʾˁʽˀʼʻʴˁʳʹ = (float)(ʲʻʺʹʺʲʷʾʻʺʹ - ʺˁʾˀʿʲʹʼʴˁʴ);
		ʺˁʾˀʿʲʹʼʴˁʴ = ʲʻʺʹʺʲʷʾʻʺʹ;
		ʺʼʲʾʷˀʽʷʲʹʿ = ʴʼʿʼʺʹʸʲʽʾʽ.ʵʲˁˀʲʳʼʶʾˀʺ((float)ʲʻʺʹʺʲʷʾʻʺʹ);

// ---- C5 the clock class ʴʶʺˀˀʺʹʴʿʹʷ: its divisor and its rate, 1 by default : ʴʶʺˀˀʺʹʴʿʹʷ.cs lines 18 to 20 of the ilspycmd output ----
	private float ʳʷʴʺʳʹʷʾʲʷʾ;

	private float ˁʻʷʵˁʳʿʳʺʸʷ = 1f;

// ---- C5 the clock class, the divisor setter : ʴʶʺˀˀʺʹʴʿʹʷ.cs lines 44 to 50 of the ilspycmd output ----
	public float ʽʸʿʹʾʹʼʹʶʽʹ
	{
		set
		{
			ʳʷʴʺʳʹʷʾʲʷʾ = value;
		}
	}

// ---- C5 the clock class, its three readouts : ʴʶʺˀˀʺʹʴʿʹʷ.cs lines 92 to 98 of the ilspycmd output ----
	public bool ʶʿʳʷʴʹˁʾʺʾˀ => ʲʻʺʹʺʲʷʾʻʺʹ >= 0.0;

	public double ʾʻʹʵʹʵʶʺʿˁʸ => ʲʻʺʹʺʲʷʾʻʺʹ + ʿʻʽʼʹʸˀʶʲʶʲ;

	public double ˁʶʾʹʽʾʼʳʷʺʽ => ʾʻʹʵʹʵʶʺʿˁʸ - ʷʲʻʺʽʳʺʺʾʹʴ;

	public double ˁʷʹʷʴʳʾʲʼʸʻ => ʾʻʹʵʹʵʶʺʿˁʸ + ʾˀʺʷʹˀʶʾˁʸʻ;

// ---- C6 the clock tick ʿʸʶˁʷˁʼʳʶʴʸ, which C4 calls: the audio position over the divisor, followed by Time.deltaTime times the rate : ʴʶʺˀˀʺʹʴʿʹʷ.cs lines 106 to 139 of the ilspycmd output ----
	public void ʿʸʶˁʷˁʼʳʶʴʸ()
	{
		ʺʲʽʾʻˀʽʶʵʿʾ = BassAudioManager.ʷʲʺʷʻʳʾʶˁˀʷ.ʴʴʳʶʺʴʾʲʸʶˀ() / (double)ʳʷʴʺʳʹʷʾʲʷʾ;
		ʲʻʺʹʺʲʷʾʻʺʹ += Time.deltaTime * ˁʻʷʵˁʳʿʳʺʸʷ;
		float value = (float)(ʺʲʽʾʻˀʽʶʵʿʾ - ʲʻʺʹʺʲʷʾʻʺʹ);
		if (!ʽʳʼʲʳˀʳʼʾʸʽ && ʲʻʺʹʺʲʷʾʻʺʹ > BassAudioManager.ʷʲʺʷʻʳʾʶˁˀʷ.ʹʾʹʸʲʲʾʼʵʹʼ / (double)ʳʷʴʺʳʹʷʾʲʷʾ - 1.0)
		{
			ʽʳʼʲʳˀʳʼʾʸʽ = true;
		}
		if (!ʽʳʼʲʳˀʳʼʾʸʽ)
		{
			if (Math.Abs(ʾʳʷʶʷʲʿʼʶʸʴ) >= Math.Abs(ʷʳʲʴʸʺʺʶʸʲʻ))
			{
				ʷʳʲʴʸʺʺʶʸʲʻ -= ʾʳʷʶʷʲʿʼʶʸʴ;
				ʾʳʷʶʷʲʿʼʶʸʴ = 0f;
				ʺʶʿʹˁʸʿʼʸʺʽ = true;
			}
			if (ʺʲʽʾʻˀʽʶʵʿʾ > 0.0 && ʺʶʿʹˁʸʿʼʸʺʽ && Math.Abs(value) > ʽʻʺʼʲʸʶˁʵʸʾ && BassAudioManager.ʷʲʺʷʻʳʾʶˁˀʷ.ˀʴʵʴʺʲʻʻʻʸʸ && ʺʲʽʾʻˀʽʶʵʿʾ < BassAudioManager.ʷʲʺʷʻʳʾʶˁˀʷ.ʹʾʹʸʲʲʾʼʵʹʼ / (double)ʳʷʴʺʳʹʷʾʲʷʾ - 1.0)
			{
				ʷʳʲʴʸʺʺʶʸʲʻ = value;
				ʺʶʿʹˁʸʿʼʸʺʽ = false;
			}
			if (Math.Abs(ʷʳʲʴʸʺʺʶʸʲʻ) > ʽʻʺʼʲʸʶˁʵʸʾ)
			{
				float num = ʷʳʲʴʸʺʺʶʸʲʻ * Time.deltaTime;
				ʾʳʷʶʷʲʿʼʶʸʴ += num;
				ʲʻʺʹʺʲʷʾʻʺʹ += num;
			}
			else
			{
				ʺʶʿʹˁʸʿʼʸʺʽ = true;
			}
		}
	}

// ---- C7 the audio position ʴʴʳʶʺʴʾʲʸʶˀ, read through an audio wrapper class outside Assembly-CSharp (not decompiled) : BassAudioManager.cs lines 740 to 748 of the ilspycmd output ----
	public double ʴʴʳʶʺʴʾʲʸʶˀ()
	{
		if (ʶˁʼʹʾʳʳʸʹʶʽ)
		{
			long ʼʺˀʹʷʵʸˀˀʳʳ = ʷʹʹʻʳʶʶʶʶˁʿ.ʷʺʽʲʵʵʹˀʹˁʷ(ʴʵʳˀʴʼʴʺʷʷʽ);
			return ʷʹʹʻʳʶʶʶʶˁʿ.ˀʵʺʼʾʷˀˀʿˁʿ(ʴʵʳˀʴʼʴʺʷʷʽ, ʼʺˀʹʷʵʸˀˀʳʳ) + ʴʹʷʾʿʾʹʵʿʷʻ;
		}
		return 0.0;
	}

// ---- C8 song end: the audio length over the song speed : GameManager.cs lines 412 to 412 of the ilspycmd output ----
			ʷʾˀʲʸˀʺˁʻʲʼ = BassAudioManager.ʷʲʺʷʻʳʾʶˁˀʷ.ʹʾʹʸʲʲʾʼʵʹʼ / (double)ʹʺʽˁʽˁˀʼʶʷʼ.ʳʷʴʺʳʹʷʾʲʷʾ.GetFloatPercent;

// ---- C9 the rate is changed only by buttons 31 and 32, which are not reached when the song speed is not 100 : GameManager.cs lines 2347 to 2375 of the ilspycmd output ----
			}
			if (ʹʺʽˁʽˁˀʼʶʷʼ.ʳʷʴʺʳʹʷʾʲʷʾ.CurrentValue != 100)
			{
				break;
			}
			if (item.ʾʵʻʳʷʴʼʲʻʶʷ.GetButtonDown(31))
			{
				if (ʻʽʻʳʽʽʴʶʷʼʺ >= 4900f)
				{
					break;
				}
				ˁʻʷʵˁʳʿʳʺʸʷ += 0.05f;
				ʻʽʻʳʽʽʴʶʷʼʺ += 5f;
				ʼˁʸʹʽʺˀʺˁʾʻ.ʶʻʿʶʴˁʷˁʼʲʳ = ˁʻʷʵˁʳʿʳʺʸʷ;
				ʸʿʽˁʸʹˀʵʺʿʵ();
				ʳʺˀˁʴˀʳˁʴʴʽ.ʿʸʳʻʶˁʹʿʹˁʽ(100 + (int)ʻʽʻʳʽʽʴʶʷʼʺ);
			}
			else if (item.ʾʵʻʳʷʴʼʲʻʶʷ.GetButtonDown(32))
			{
				if (ʻʽʻʳʽʽʴʶʷʼʺ <= -90f)
				{
					break;
				}
				ˁʻʷʵˁʳʿʳʺʸʷ -= 0.05f;
				ʻʽʻʳʽʽʴʶʷʼʺ -= 5f;
				ʼˁʸʹʽʺˀʺˁʾʻ.ʶʻʿʶʴˁʷˁʼʲʳ = ˁʻʷʵˁʳʿʳʺʸʷ;
				ʸʿʽˁʸʹˀʵʺʿʵ();
				ʳʺˀˁʴˀʳˁʴʴʽ.ʿʸʳʻʶˁʹʿʹˁʽ(100 + (int)ʻʽʻʳʽʽʴʶʷʼʺ);
			}

// ---- S1 the drain base, set by ʿʻʷʳʶʽʼʹˀˁʻ, which C4's Update calls: eight measures in the file's resolution : GameManager.cs lines 2120 to 2127 of the ilspycmd output ----
	private void ʿʻʷʳʶʽʼʹˀˁʻ()
	{
		if (!(ʲʻʺʹʺʲʷʾʻʺʹ <= 0.0))
		{
			ˀʸʼʹʽʺʸˀʷʽʵ ˀʸʼʹʽʺʸˀʷʽʵ2 = ʴʼʿʼʺʹʸʲʽʾʽ.ʴʵʿʹʹʾʿʳˀʵʼ(ʺʼʲʾʷˀʽʷʲʹʿ);
			ˀʲʸʼʽʶʴʸʴʶʼ = (int)(ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ * 4f / (float)ˀʸʼʹʽʺʸˀʷʽʵ2.ʲʲˀʵʼʿʳʸʾʾʴ * (float)ˀʸʼʹʽʺʸˀʷʽʵ2.ʶʶˁʿʷʾʹʸʴʷʲ) * 8;
		}
	}

// ---- S2 the drain per frame ʶʽʿʲʵʺʼʺʹʵʲ: chart ticks since the last frame over S1's base : GameManager.cs lines 1642 to 1649 of the ilspycmd output ----
	public float ʶʽʿʲʵʺʼʺʹʵʲ()
	{
		if (!ˀʴʵʴʺʲʻʻʻʸʸ)
		{
			return 0f;
		}
		return (float)(int)(ʺʼʲʾʷˀʽʷʲʹʿ - ʻʴʶʼʽʵˁʳʼʳʺ) / (float)ˀʲʸʼʽʶʴʸʴʶʼ;
	}

// ---- S3 the drain is applied while Star Power is active : BasePlayer.cs lines 310 to 314 of the ilspycmd output ----
		if (ˁʸʴʵʳʾʽʿʹʷʿ)
		{
			ʵʳʴʻʲʻʹʿʻʳʴ += ˀʹʳʴʸʺʳʴʷʵʽ.ˀʾˁʽˀʼʻʴˁʳʹ;
			ʼʲʴʲˀʺʲʸʺʹʵ(ˀʹʳʴʸʺʳʴʷʵʽ.ʶʽʿʲʵʺʼʺʹʵʲ());
		}

// ---- S4 the drain method ʼʲʴʲˀʺʲʸʺʹʵ : BasePlayer.cs lines 279 to 290 of the ilspycmd output ----
	protected void ʼʲʴʲˀʺʲʸʺʹʵ(float ʴʷˀʳʿʴʿʹʴʾʸ)
	{
		if (!ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.isRemotePlayer)
		{
			ʹˀʼʵʳˀʻʼʵʿʵ -= ʴʷˀʳʿʴʿʹʴʾʸ;
			if (ʹˀʼʵʳˀʻʼʵʿʵ <= 0f)
			{
				ʹʻʶʾʶʴʿʲʷˀʹ();
			}
			ʳʿʿˀʹʻʳʴʳʹʹ.ʸʿʲˁʴʾˁʴʻʳʼ(ʹˀʼʵʳˀʻʼʵʿʵ, ˁʸʴʵʳʾʽʿʹʷʿ);
		}
	}

// ---- S5 a finished phrase adds a fixed quarter of the meter : BasePlayer.cs lines 912 to 916 of the ilspycmd output ----
			if (!GlobalVariables.ʷʲʺʷʻʳʾʶˁˀʷ.ʽʹʷʵʿʼʿʺʾʶʴ)
			{
				ʼʽʶʽʴʳˀˁˁʼʸ.ˀʷˁʵʹʳʿʵʼʹˀ &= ~ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.STAR_POWER_END;
				ʸʸʳʽʲʻʿʸʳʸʽ(0.25f, ˀʵʼˁʸʵʻʽʹʻˁ: true);
				ˀʴʵʸʶʸʲʾʶʵʲ = true;

// ---- S6 the sustain gain base ʿˁˁʾʸʴʸʿʶʽʺ, 30 times the file's resolution, set at chart load : GameManager.cs lines 1418 to 1420 of the ilspycmd output ----
		ˁʷʴʲʻʿʸʴʳʽʿ = Mathf.Max((int)ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ, 25) / 25;
		ʻʸʴʳʹʼʹʷʲʹʹ = (uint)ʴʼʿʼʺʹʸʲʽʾʽ.ˀʻʺʷʾʿʿʻʼʲʿ / 4;
		ʿˁˁʾʸʴʸʿʶʽʺ = 30 * (int)ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ;

// ---- S7 the sustain gain per frame ʾˁʿʽʺʸʶʵʵʸʷ: chart ticks since the last frame over S6's base : GameManager.cs lines 2166 to 2173 of the ilspycmd output ----
	public float ʾˁʿʽʺʸʶʵʵʸʷ()
	{
		if (!ˀʴʵʴʺʲʻʻʻʸʸ)
		{
			return 0f;
		}
		return (float)(int)(ʺʼʲʾʷˀʽʷʲʹʿ - ʻʴʶʼʽʵˁʳʼʳʺ) / (float)ʿˁˁʾʸʴʸʿʶʽʺ;
	}

// ---- S8 the sustain gain's only caller is guitar code : BaseGuitarPlayer.cs lines 337 to 337 of the ilspycmd output ----
			ʸʸʳʽʲʻʿʸʳʸʽ(ˀʹʳʴʸʺʳʴʷʵʽ.ʾˁʿʽʺʸʶʵʵʸʷ());

// ---- T1 adding a tempo: its start second is counted with the timing resolution, the one times the song speed : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 636 to 651 of the ilspycmd output ----

	public void ʼʺʾʼˁʼʹˁˁʷʻ(ʼʹʵˁʸʳʾʺˁʳʽ ˀˁʿʴʺˀʻʵˁʹʼ)
	{
		ˀˁʿʴʺˀʻʵˁʹʼ.ʴʼʿʼʺʹʸʲʽʾʽ = this;
		int count = ʼʿʵʸʾʼʸʿʼˀʺ.Count;
		float ʸʼʻʳʷʾʹʿˁʴʸ;
		if (count == 0 && ˀˁʿʴʺˀʻʵˁʹʼ.ʳʿʹʺʵʹʷʼʼʸʸ == 0)
		{
			ʸʼʻʳʷʾʹʿˁʴʸ = 0f;
		}
		else
		{
			ʼʹʵˁʸʳʾʺˁʳʽ ʼʹʵˁʸʳʾʺˁʳʽ2 = ʼʿʵʸʾʼʸʿʼˀʺ[count - 1];
			ʸʼʻʳʷʾʹʿˁʴʸ = ʼʹʵˁʸʳʾʺˁʳʽ2.ʸʼʻʳʷʾʹʿˁʴʸ + ʽˀˀʻʹʶˁʽˁˁʽ(ʼʹʵˁʸʳʾʺˁʳʽ2.ʳʿʹʺʵʹʷʼʼʸʸ, ˀˁʿʴʺˀʻʵˁʹʼ.ʳʿʹʺʵʹʷʼʼʸʸ, ˀʻʺʷʾʿʿʻʼʲʿ, (float)ʼʹʵˁʸʳʾʺˁʳʽ2.ʶʲʵʻˁʻʺʴʿʺʵ / 1000f);
		}
		ˀˁʿʴʺˀʻʵˁʹʼ.ʸʼʻʳʷʾʹʿˁʴʸ = ʸʼʻʳʷʾʹʿˁʴʸ;
